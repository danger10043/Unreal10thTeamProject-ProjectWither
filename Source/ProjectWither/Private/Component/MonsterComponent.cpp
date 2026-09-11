// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MonsterComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/StatComponent.h"
#include "Data/ItemDropTable.h"
#include "DataAsset/ItemDataAsset.h"
#include "Item/PickupItem.h"
#include "Monster/MonsterAIController.h"
#include "Monster/MonsterProjectile.h"
#include "Interface/PlayerInterface.h"
#include "Interface/StatComponentUserInterface.h"
#include "Framework/SubSystem/ObjectPoolSubsystem.h"
#include "DataAsset/MonsterDataAsset.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

UMonsterComponent::UMonsterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		SpawnLocation = Owner->GetActorLocation();
		StatComponent = Owner->FindComponentByClass<UStatComponent>();
		if (StatComponent)
		{
			StatComponent->OnHealthZero.AddUniqueDynamic(this, &UMonsterComponent::HandleDeath);
		}

		ApplyMonsterData();

		CachePawnCollisionResponses();
	}
}

void UMonsterComponent::ApplyMonsterData()
{
	if (!IsValid(MonsterData))
	{
		return;
	}

	MonsterId = MonsterData->MonsterId;
	AllowRange = FMath::Max(0.0f, MonsterData->BaseSettings.AllowRange);
	AttackRange = FMath::Max(0.0f, MonsterData->BaseSettings.AttackRange);
	AttackCooldown = FMath::Max(0.0f, MonsterData->BaseSettings.AttackCooldown);
	DespawnPolicy = MonsterData->DespawnPolicy;
	DespawnDelay = FMath::Max(0.0f, MonsterData->DespawnDelay);
	AttackMontage = MonsterData->AttackMontage;
	HitReactMontage = MonsterData->HitReactMontage;
	ParriedMontage = MonsterData->ParriedMontage;
	DeathMontage = MonsterData->DeathMontage;
	SearchMontage = MonsterData->SearchMontage;
	// 데이터 에셋에 값이 지정된 경우 우선 사용하고, 비어 있으면 몬스터 BP에 설정된 값을 유지한다.
	if (IsValid(MonsterData->ItemDropTable))
	{
		ItemDropTable = MonsterData->ItemDropTable;
	}
	if (MonsterData->ItemPickupClass)
	{
		ItemPickupClass = MonsterData->ItemPickupClass;
	}

	if (IsValid(StatComponent))
	{
		StatComponent->ConfigureStats(
			MonsterData->MaxHealth,
			MonsterData->MaxStamina,
			MonsterData->MinAttackPower,
			MonsterData->MaxAttackPower,
			MonsterData->DefensePower);
		StatComponent->SetCombatMultipliers(
			MonsterData->BaseSettings.AttackPowerMultiplier,
			MonsterData->BaseSettings.DefenseMultiplier);
	}

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =
			FMath::Max(0.0f, MonsterData->BaseSettings.MoveSpeed);
	}
}

void UMonsterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthZero.RemoveDynamic(this, &UMonsterComponent::HandleDeath);
	}
	ClearRuntimeTimers();

	DisableAllAttackHitboxes();

	for (const auto& Entry : AttackHitboxes)
	{
		if (IsValid(Entry.Value.Get()))
		{
			Entry.Value->OnComponentBeginOverlap.RemoveDynamic(
				this,
				&UMonsterComponent::OnAttackHitboxOverlap);
		}
	}

	AttackHitboxes.Reset();

	Super::EndPlay(EndPlayReason);
}

float UMonsterComponent::ApplyMonsterDamage(float Damage)
{
	if (bIsDead || bDamageLocked || !IsValid(StatComponent))
	{
		return 0.0f;
	}

	const float AppliedDamage = StatComponent->ApplyDamage(Damage);

	if (!bIsDead && AppliedDamage > 0.0f)
	{
		PlayHitReaction();
	}

	return AppliedDamage;
}

void UMonsterComponent::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	CancelAttack();
	LockMovementForMontage();

	MonsterState = EMonsterState::Dead;
	ClearTarget();

	ClearRuntimeTimers();

	DisableAllAttackHitboxes();

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AAIController* AIController =
			Cast<AAIController>(Pawn->GetController()))
		{
			AIController->StopMovement();

			if (UBrainComponent* Brain =
				AIController->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Monster died"));
			}
		}
	}

	SetDeadCollision(true);

	CalculateDrops();
	DropItems();
	GrantGoldReward();

	OnMonsterDied.Broadcast();

	PlayDeathMontage();
}

void UMonsterComponent::SetMonsterState(EMonsterState NewState)
{
	if (!bIsDead) MonsterState = NewState;
}

void UMonsterComponent::SetTarget(AActor* NewTarget)
{
	if (!bIsDead) TargetActor = NewTarget;
}

void UMonsterComponent::ClearTarget()
{
	TargetActor = nullptr;
}

AActor* UMonsterComponent::GetTargetActor() const
{
	return IsValid(TargetActor) ? TargetActor.Get() : nullptr;
}

float UMonsterComponent::GetDistanceToTarget() const
{
	if (!IsValid(TargetActor) || !GetOwner()) return -1.0f;

	return GetOwner()->GetDistanceTo(TargetActor);
}

void UMonsterComponent::CalculateDrops()
{
	DropItem.Reset();
	if (!ItemDropTable) return;

	TArray<FItemDropTable*> AllRows;
	ItemDropTable->GetAllRows<FItemDropTable>(
		TEXT("UMonsterComponent::CalculateDrops"),
		AllRows
	);
	for (FItemDropTable* Row : AllRows)
	{
		// 필수 데이터 확인
		if (!Row || !Row->ItemData) continue;

		// 드랍 확률 체크
		if (FMath::FRand() > Row->DropRate) continue;

		int32 Quantity = FMath::RandRange(Row->MinQuantity, Row->MaxQuantity);

		DropItem.FindOrAdd(Row->ItemData) += Quantity;
	}
}

void UMonsterComponent::DropItems()
{
	if (!GetOwner()) return;

	if (!ItemPickupClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: ItemPickupClass가 없어 드랍 아이템을 스폰할 수 없습니다."), *GetOwner()->GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TPair<TObjectPtr<UItemDataAsset>, int32>& Drop : DropItem)
	{
		if (!Drop.Key || Drop.Value <= 0)
		{
			continue;
		}

		FVector DropLocation =
			GetOwner()->GetActorLocation() +
			FVector(
				FMath::RandRange(-DropRange, DropRange),
				FMath::RandRange(-DropRange, DropRange),
				DropHeight
			);

		// 높은 위치에서 죽더라도 픽업이 공중에 남지 않도록 월드 지형까지 내린다.
		FHitResult FloorHit;
		FCollisionQueryParams FloorTraceParams(SCENE_QUERY_STAT(PickupFloorTrace), false, GetOwner());
		const FVector TraceStart = DropLocation + FVector(0.0f, 0.0f, 200.0f);
		const FVector TraceEnd = DropLocation - FVector(0.0f, 0.0f, 5000.0f);
		const FCollisionObjectQueryParams FloorObjectTypes(ECC_TO_BITFIELD(ECC_WorldStatic));

		if (World->LineTraceSingleByObjectType(
			FloorHit,
			TraceStart,
			TraceEnd,
			FloorObjectTypes,
			FloorTraceParams))
		{
			DropLocation = FloorHit.ImpactPoint + FVector(0.0f, 0.0f, 5.0f);
		}

		const FTransform PickupTransform(FRotator::ZeroRotator, DropLocation);
		APickupItem* Pickup = World->SpawnActorDeferred<APickupItem>(
			ItemPickupClass,
			PickupTransform,
			GetOwner(),
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (Pickup)
		{
			FItemInstance DroppedItem;
			DroppedItem.ItemData = Drop.Key;
			DroppedItem.Quantity = Drop.Value;

			Pickup->InitializePickup(DroppedItem);
			Pickup->FinishSpawning(PickupTransform);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: %s 픽업 액터 스폰에 실패했습니다."),
				*GetOwner()->GetName(), *Drop.Key->GetName());
		}
	}

	DropItem.Reset();
}

void UMonsterComponent::GrantGoldReward()
{
	if (!IsValid(MonsterData) || MonsterData->GoldReward <= 0)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(PlayerPawn))
	{
		return;
	}

	if (UInventoryComponent* InventoryComponent =
		PlayerPawn->FindComponentByClass<UInventoryComponent>())
	{
		InventoryComponent->AddGold(MonsterData->GoldReward);
	}
}

bool UMonsterComponent::IsInAttackRange()
{
	if (!IsValid(TargetActor) || !IsValid(GetOwner()))
	{
		bWasInAttackRange = false;
		return false;
	}

	const float Distance = GetDistanceToTarget();

	// 경계선 근처에서 짧은 시간에 여러 번 뒤집히면 BT 블랙보드 옵저버가
	// 전환을 놓칠 수 있어서, 진입(AttackRange)과 이탈(AttackRange + Hysteresis)
	// 기준을 다르게 둬서 경계 근처의 미세한 움직임으로는 값이 안 뒤집히게 함
	const float EffectiveRange = bWasInAttackRange
		? (AttackRange + AttackRangeHysteresis)
		: AttackRange;

	const bool bResult = Distance >= 0.0f && Distance <= EffectiveRange;
	bWasInAttackRange = bResult;

	return bResult;
}

bool UMonsterComponent::CanAttack()
{
	if (bIsDead || !bCanAttack || bCombatLocked)
	{
		return false;
	}

	if (!IsValid(StatComponent) || StatComponent->IsHealthZero())
	{
		return false;
	}

	if (MonsterState == EMonsterState::Attack ||
		MonsterState == EMonsterState::Hit ||
		MonsterState == EMonsterState::Dead)
	{
		return false;
	}

	return IsInAttackRange();
}

bool UMonsterComponent::Attack()
{
	if (!CanAttack())
	{
		return false;
	}

	UAnimMontage* SelectedMontage = SelectAttackMontage();
	if (!IsValid(SelectedMontage)) return false;

	USkeletalMeshComponent* Mesh =
		GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance =
		IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance)) return false;

	const FName SelectedSection = SelectAttackSection(SelectedMontage);
	if (SelectedSection.IsNone()) return false;

	const EMonsterState PreviousState = MonsterState;

	bCanAttack = false;
	SetMonsterState(EMonsterState::Attack);
	LockMovementForMontage();

	const float PlayedLength =
		AnimInstance->Montage_Play(SelectedMontage);


	if (PlayedLength <= 0.0f)
	{
		// 재생 실패 시 공격 잠금 복구
		UnlockMovementAfterMontage();
		bCanAttack = true;
		SetMonsterState(PreviousState);
		return false;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UMonsterComponent::OnAttackMontageEnded);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		SelectedMontage);

	AnimInstance->Montage_JumpToSection(
		SelectedSection, SelectedMontage);

	// 재생에 성공한 경우에만 직전 공격으로 기록
	ActiveAttackMontage = SelectedMontage;
	LastAttackSection = SelectedSection;

	return true;
}

bool UMonsterComponent::IsAttackMontagePlaying() const
{
	if (!IsValid(ActiveAttackMontage))
	{
		return false;
	}

	const AActor* Owner = GetOwner();
	const USkeletalMeshComponent* Mesh =
		IsValid(Owner) ? Owner->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	const UAnimInstance* AnimInstance =
		IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	return IsValid(AnimInstance) &&
		AnimInstance->Montage_IsPlaying(ActiveAttackMontage);
}

void UMonsterComponent::FinishAttack()
{
	DisableAllAttackHitboxes();
	ActiveAttackMontage = nullptr;
	if (!bIsDead)
	{
		UnlockMovementAfterMontage();
	}

	if (bIsDead) return;

	// Hit 등 다른 상태로 전환됐다면 유지
	if (MonsterState == EMonsterState::Attack)
	{
		SetMonsterState(
			IsValid(GetTargetActor())
			? EMonsterState::Chase
			: EMonsterState::Idle);
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 중복 종료 호출로 쿨타임이 계속 연장되지 않게 방지
	if (bCanAttack ||
		World->GetTimerManager().IsTimerActive(AttackCooldownTimerHandle))
	{
		return;
	}

	if (AttackCooldown <= 0.0f)
	{
		ResetAttackCooldown();
		return;
	}

	World->GetTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&UMonsterComponent::ResetAttackCooldown,
		AttackCooldown,
		false);
}

void UMonsterComponent::ResetAttackCooldown()
{
	if (bIsDead) return;

	bCanAttack = true;

}

void UMonsterComponent::ApplyAttackDamage(AActor* HitTarget, float AttackMultiplier)
{
	if (bIsDead || bCombatLocked || !IsValid(HitTarget) || !IsValid(StatComponent))
	{
		return;
	}

	if (!HitTarget->Implements<UStatComponentUserInterface>())
	{
		return;
	}

	UStatComponent* TargetStat = IStatComponentUserInterface::Execute_GetStatComponent(HitTarget);

	if (!IsValid(TargetStat) || TargetStat->IsHealthZero())
	{
		return;
	}

	const float MinPower = FMath::Max(0.0f, StatComponent->GetMinAttackPower());

	const float MaxPower = FMath::Max(MinPower, StatComponent->GetMaxAttackPower());

	const float BaseDamage = FMath::FRandRange(MinPower, MaxPower);

	const float RawDamage = BaseDamage * FMath::Max(0.0f, AttackMultiplier);

	UGameplayStatics::ApplyDamage(
		HitTarget,
		RawDamage,
		GetOwner()->GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass());
}

void UMonsterComponent::FireProjectileAtTarget()
{
	if (bIsDead || !IsValid(ProjectileClass))
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !IsValid(TargetActor))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();

	const FVector MuzzleLocation =
		(IsValid(Mesh) && Mesh->DoesSocketExist(ProjectileMuzzleSocketName))
		? Mesh->GetSocketLocation(ProjectileMuzzleSocketName)
		: Owner->GetActorLocation();

	const FVector LaunchDirection = (TargetActor->GetActorLocation() - MuzzleLocation).GetSafeNormal();

	if (LaunchDirection.IsNearlyZero())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Owner->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMonsterProjectile* Projectile = World->SpawnActor<AMonsterProjectile>(
		ProjectileClass,
		MuzzleLocation,
		LaunchDirection.Rotation(),
		SpawnParams);

	if (IsValid(Projectile))
	{
		Projectile->InitializeProjectile(this, LaunchDirection, ProjectileAttackMultiplier);
	}
}

void UMonsterComponent::RegisterAttackHitbox(FName HitboxName, UPrimitiveComponent* Hitbox)
{
	if (HitboxName.IsNone() || !IsValid(Hitbox))
	{
		return;
	}

	if (Hitbox->GetOwner() != GetOwner())
	{
		return;
	}

	if (AttackHitboxes.Contains(HitboxName))
	{
		return;
	}

	// 같은 콜리전을 다른 이름으로 중복 등록 방지
	for (const auto& Entry : AttackHitboxes)
	{
		if (Entry.Value.Get() == Hitbox)
		{
			return;
		}
	}

	Hitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Hitbox->SetGenerateOverlapEvents(true);
	Hitbox->SetCollisionObjectType(ECC_WorldDynamic);
	Hitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	Hitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Hitbox->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UMonsterComponent::OnAttackHitboxOverlap);

	AttackHitboxes.Add(HitboxName, Hitbox);

}

void UMonsterComponent::BeginAttackHitWindow(FName HitboxName)
{
	if (bIsDead || bCombatLocked || MonsterState != EMonsterState::Attack)
	{
		return;
	}

	// 이전 구간 종료
	DisableAllAttackHitboxes();

	const TObjectPtr<UPrimitiveComponent>* Found =
		AttackHitboxes.Find(HitboxName);

	if (!Found || !IsValid(Found->Get()))
	{
		return;
	}

	UPrimitiveComponent* Hitbox = Found->Get();

	// 활성화 순간 Overlap이 발생할 수 있으므로 먼저 기록
	HitActors.Reset();
	ActiveHitboxName = HitboxName;
	ActiveAttackHitbox = Hitbox;

	Hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 활성화 시 이미 겹쳐 있는 대상도 검사
	TArray<AActor*> OverlappingActors;
	Hitbox->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		ProcessAttackOverlap(Actor);
	}
}

void UMonsterComponent::EndAttackHitWindow(FName HitboxName)
{
	const TObjectPtr<UPrimitiveComponent>* Found =
		AttackHitboxes.Find(HitboxName);

	if (!Found || !IsValid(Found->Get()))
	{
		return;
	}

	UPrimitiveComponent* Hitbox = Found->Get();

	if (ActiveAttackHitbox.Get() == Hitbox)
	{
		ActiveAttackHitbox = nullptr;
		ActiveHitboxName = NAME_None;
		HitActors.Reset();
	}

	Hitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UMonsterComponent::CancelAttack()
{
	DisableAllAttackHitboxes();

	if (AActor* Owner = GetOwner())
	{
		if (USkeletalMeshComponent* Mesh =
			Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				if (IsValid(ActiveAttackMontage) &&
					AnimInstance->Montage_IsPlaying(ActiveAttackMontage))
				{
					AnimInstance->Montage_Stop(0.15f, ActiveAttackMontage);
				}
			}
		}
	}
}

void UMonsterComponent::PlayHitReaction()
{
	if (bIsDead || bCombatLocked)
	{
		return;
	}

	// 공격 중에는 체력만 감소하고 공격은 유지
	if (MonsterState == EMonsterState::Attack)
	{
		return;
	}

	PlayReactionMontage(HitReactMontage);
}

void UMonsterComponent::HandleParried()
{
	if (bIsDead || bCombatLocked)
	{
		return;
	}

	CancelAttack();
	PlayReactionMontage(ParriedMontage);
}

void UMonsterComponent::ActivateFromPool()
{
	const AActor* Owner = GetOwner();
	ResetForReuse(
		IsValid(Owner)
		? Owner->GetActorLocation()
		: FVector::ZeroVector);
}

void UMonsterComponent::DeactivateForPool()
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AMonsterAIController* AI = Cast<AMonsterAIController>(Pawn->GetController()))
		{
			AI->StopAI();
		}
	}
	DisableAllAttackHitboxes();
	StopAllMontages();

	ClearRuntimeTimers();
	ClearTarget();
}

void UMonsterComponent::ResetForReuse(const FVector& NewSpawnLocation)
{
	ClearRuntimeTimers();
	StopAllMontages();
	UnlockMovementAfterMontage();

	// 먼저 MonsterState를 Idle로 초기화
	ResetRuntimeState();

	SpawnLocation = NewSpawnLocation;

	// 그다음 AnimBP 상태 머신 초기화
	ResetAnimation();

	SetDeadCollision(false);
	RestartAI();
}

bool UMonsterComponent::PlaySearchAnimation()
{
	if (bIsDead || bCombatLocked || !IsValid(SearchMontage) || !IsValid(GetOwner()))
	{
		return false;
	}

	if (MonsterState == EMonsterState::Attack ||
		MonsterState == EMonsterState::Hit ||
		MonsterState == EMonsterState::Dead)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance))
	{
		return false;
	}

	const EMonsterState PreviousState = MonsterState;
	SetMonsterState(EMonsterState::Search);

	const float PlayedLength = AnimInstance->Montage_Play(SearchMontage);

	if (PlayedLength <= 0.0f)
	{
		SetMonsterState(PreviousState);
		return false;
	}

	LockMovementForMontage();

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UMonsterComponent::OnSearchMontageEnded);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		SearchMontage);

	return true;
}

void UMonsterComponent::CancelSearch()
{
	if (!IsValid(GetOwner()) || !IsValid(SearchMontage))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (IsValid(AnimInstance) && AnimInstance->Montage_IsPlaying(SearchMontage))
	{
		AnimInstance->Montage_Stop(0.15f, SearchMontage);
	}

	if (!bIsDead && MonsterState == EMonsterState::Search)
	{
		SetMonsterState(IsValid(GetTargetActor()) ? EMonsterState::Chase : EMonsterState::Idle);
	}
}

void UMonsterComponent::ScheduleFinishDeath(float MinimumDelay)
{
	if (DespawnPolicy == EMonsterDespawnPolicy::KeepCorpse)
	{
		return;
	}

	const float EffectiveDelay = FMath::Max(DespawnDelay, MinimumDelay);

	if (EffectiveDelay <= 0.0f)
	{
		FinishDeath();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DespawnTimerHandle,
			this,
			&UMonsterComponent::FinishDeath,
			EffectiveDelay,
			false
		);
	}
}

UAnimMontage* UMonsterComponent::SelectAttackMontage() const
{
	if (bUseOnlyAdditionalAttackMontage && IsValid(AdditionalAttackMontage))
	{
		return AdditionalAttackMontage.Get();
	}

	if (IsValid(AttackMontage) && IsValid(AdditionalAttackMontage))
	{
		return FMath::RandBool() ? AttackMontage.Get() : AdditionalAttackMontage.Get();
	}

	return IsValid(AttackMontage) ? AttackMontage.Get() : AdditionalAttackMontage.Get();
}

FName UMonsterComponent::SelectAttackSection(UAnimMontage* Montage) const
{
	if (!IsValid(Montage))
	{
		return NAME_None;
	}

	TArray<FName> Candidates;

	for (int32 Index = 0; Index < Montage->GetNumSections(); Index++)
	{
		const FName SectionName = Montage->GetSectionName(Index);

		if (!SectionName.ToString().StartsWith(AttackSectionPrefix))
		{
			continue;
		}

		if (SectionName == LastAttackSection)
		{
			continue;
		}

		Candidates.Add(SectionName);
	}

	// 공격 섹션이 하나일 경우
	if (Candidates.IsEmpty())
	{
		for (int32 Index = 0;
			Index < Montage->GetNumSections();
			++Index)
		{
			const FName SectionName =
				Montage->GetSectionName(Index);

			if (SectionName.ToString().StartsWith(
				AttackSectionPrefix))
			{
				Candidates.Add(SectionName);
			}
		}
	}

	// 그래도 비었으면
	if (Candidates.IsEmpty())
	{
		return NAME_None;
	}

	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

void UMonsterComponent::DisableAllAttackHitboxes()
{
	ActiveAttackHitbox = nullptr;
	ActiveHitboxName = NAME_None;
	HitActors.Reset();

	for (const auto& Entry : AttackHitboxes)
	{
		if (IsValid(Entry.Value.Get()))
		{
			Entry.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void UMonsterComponent::OnAttackHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != ActiveAttackHitbox.Get())
	{
		return;
	}

	ProcessAttackOverlap(OtherActor);
}

void UMonsterComponent::ProcessAttackOverlap(AActor* OtherActor)
{
	if (bIsDead || bCombatLocked ||
		MonsterState != EMonsterState::Attack ||
		!IsValid(ActiveAttackHitbox))
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	if (!OtherActor->Implements<UPlayerInterface>() ||
		!OtherActor->Implements<UStatComponentUserInterface>())
	{
		return;
	}

	UStatComponent* PlayerStat =
		IStatComponentUserInterface::Execute_GetStatComponent(OtherActor);

	if (!IsValid(PlayerStat) || PlayerStat->IsHealthZero())
	{
		return;
	}

	const TWeakObjectPtr<AActor> HitActor(OtherActor);

	if (HitActors.Contains(HitActor))
	{
		return;
	}

	HitActors.Add(HitActor);

	float AttackMultiplier = 1.0f;

	if (ActiveHitboxName == TEXT("Mouth"))
	{
		AttackMultiplier = 1.2f;
	}
	else if (ActiveHitboxName == TEXT("LeftFoot"))
	{
		AttackMultiplier = 1.0f;
	}
	else if (ActiveHitboxName == TEXT("RightFoot"))
	{
		AttackMultiplier = 1.0f;
	}

	ApplyAttackDamage(OtherActor, AttackMultiplier);
}

void UMonsterComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveAttackMontage) return;

	FinishAttack();
	OnMonsterAttackFinished.Broadcast(bInterrupted);
}

void UMonsterComponent::PlayReactionMontage(UAnimMontage* Montage)
{
	if (bIsDead || !IsValid(Montage) || !IsValid(GetOwner()))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance))
	{
		return;
	}

	const float PlayedLength = AnimInstance->Montage_Play(Montage);

	if (PlayedLength <= 0.0f)
	{
		return;
	}

	SetMonsterState(EMonsterState::Hit);
	LockMovementForMontage();

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UMonsterComponent::OnReactionMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
}

void UMonsterComponent::OnReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsDead)
	{
		return;
	}

	UnlockMovementAfterMontage();

	if (MonsterState == EMonsterState::Hit)
	{
		SetMonsterState(IsValid(GetTargetActor()) ?
			EMonsterState::Chase : EMonsterState::Idle);
	}
}

void UMonsterComponent::PlayDeathMontage()
{
	if (!IsValid(GetOwner()) || !IsValid(DeathMontage))
	{
		ScheduleFinishDeath();
		return;
	}

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance))
	{
		ScheduleFinishDeath();
		return;
	}

	const float PlayedLength = AnimInstance->Montage_Play(DeathMontage);

	if (PlayedLength <= 0.0f)
	{
		ScheduleFinishDeath();
		return;
	}

	// Auto Blend Out이 꺼진 몽타주는 종료 델리게이트가 호출되지 않을 수
	// 있으므로 재생 시작 시점에도 풀 반환/파괴를 예약한다.
	ScheduleFinishDeath(PlayedLength);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UMonsterComponent::OnDeathMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
}

void UMonsterComponent::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DeathMontage)
	{
		return;
	}

	ScheduleFinishDeath();
}

void UMonsterComponent::OnSearchMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != SearchMontage)
	{
		return;
	}

	if (!bIsDead && MonsterState == EMonsterState::Search)
	{
		UnlockMovementAfterMontage();
		SetMonsterState(IsValid(GetTargetActor()) ? EMonsterState::Chase : EMonsterState::Idle);
	}

	OnMonsterSearchFinished.Broadcast(bInterrupted);
}

void UMonsterComponent::FinishDeath()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	switch (DespawnPolicy)
	{
	case EMonsterDespawnPolicy::ReturnToPool:
	{
		UWorld* World = GetWorld();
		UObjectPoolSubsystem* PoolSubsystem =
			IsValid(World)
			? World->GetSubsystem<UObjectPoolSubsystem>()
			: nullptr;

		if (!IsValid(PoolSubsystem) ||
			!PoolSubsystem->ReturnPool(Owner))
		{
			// 풀에서 생성되지 않은 몬스터에 대한 안전 처리
			Owner->Destroy();
		}

		break;
	}

	case EMonsterDespawnPolicy::Destroy:
		Owner->Destroy();
		break;

	case EMonsterDespawnPolicy::KeepCorpse:
		// 보스 시체나 연출용 몬스터
		break;

	default:
		break;
	}
}

void UMonsterComponent::ClearRuntimeTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(AttackCooldownTimerHandle);
		TimerManager.ClearTimer(DespawnTimerHandle);
	}
}

void UMonsterComponent::ResetRuntimeState()
{
	TargetActor = nullptr;
	DropItem.Reset();
	HitActors.Reset();
	ActiveAttackHitbox = nullptr;
	ActiveHitboxName = NAME_None;
	LastAttackSection = NAME_None;

	bIsDead = false;
	bCanAttack = true;
	bCombatLocked = false;
	bDamageLocked = false;
	MonsterState = EMonsterState::Idle;

	if (AActor* Owner = GetOwner())
	{
		SpawnLocation = Owner->GetActorLocation();
	}

	DisableAllAttackHitboxes();

	if (IsValid(StatComponent))
	{
		StatComponent->ResetStat();
	}
}

void UMonsterComponent::StopAllMontages()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	USkeletalMeshComponent* Mesh =
		Owner->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance =
		IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(0.0f);
	}
}

void UMonsterComponent::CachePawnCollisionResponses()
{
	OriginalPawnCollisionResponses.Reset();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		if (IsValid(Component))
		{
			OriginalPawnCollisionResponses.Add(
				Component,
				Component->GetCollisionResponseToChannel(ECC_Pawn));
		}
	}
}

void UMonsterComponent::SetDeadCollision(bool bDeadCollision)
{
	for (auto It = OriginalPawnCollisionResponses.CreateIterator(); It; ++It)
	{
		UPrimitiveComponent* Component = It.Key().Get();
		if (!IsValid(Component))
		{
			It.RemoveCurrent();
			continue;
		}

		Component->SetCollisionResponseToChannel(
			ECC_Pawn,
			bDeadCollision ? ECR_Ignore : It.Value());
	}
}

void UMonsterComponent::RestartAI()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn))
	{
		return;
	}

	// 풀에서 생성된 Pawn에 컨트롤러가 없으면 생성
	if (!IsValid(Pawn->GetController()))
	{
		Pawn->SpawnDefaultController();
	}

	if (AMonsterAIController* MonsterAI =
		Cast<AMonsterAIController>(Pawn->GetController()))
	{
		MonsterAI->RestartAI();
		return;
	}

	if (AAIController* AIController =
		Cast<AAIController>(Pawn->GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}
}

void UMonsterComponent::ResetAnimation()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	USkeletalMeshComponent* Mesh =
		Owner->FindComponentByClass<USkeletalMeshComponent>();

	if (!IsValid(Mesh))
	{
		return;
	}

	// 이전에 애니메이션을 정지한 경우까지 복구
	Mesh->bPauseAnims = false;
	Mesh->SetComponentTickEnabled(true);

	// AnimBP 상태 머신을 Entry 상태부터 다시 시작
	Mesh->InitAnim(true);
}

void UMonsterComponent::LockMovementForMontage()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn)) return;

	if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
	{
		AIController->StopMovement();
	}

	// 이미 잠겨 있다면 최초 활성 상태를 덮어쓰지 않는다.
	if (!IsValid(LockedMontageMovement))
	{
		LockedMontageMovement = OwnerPawn->GetMovementComponent();
		if (IsValid(LockedMontageMovement))
		{
			bMontageMovementWasActive = LockedMontageMovement->IsActive();
			LockedMontageMovement->StopMovementImmediately();
		}
	}

	OwnerPawn->ConsumeMovementInputVector();
}

void UMonsterComponent::UnlockMovementAfterMontage()
{
	LockedMontageMovement = nullptr;
	bMontageMovementWasActive = false;
}
