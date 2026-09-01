// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Data/ItemDropTable.h"
#include "Item/PickupItem.h"
#include "Monster/MonsterAIController.h"
#include "Player/PlayerCharacter.h"
#include "Interface/StatComponentUserInterface.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
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
    }
}

void UMonsterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(StatComponent))
    {
        StatComponent->OnHealthZero.RemoveDynamic(this, &UMonsterComponent::HandleDeath);
    }
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
	}

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
	if (bIsDead || !IsValid(StatComponent))
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

	MonsterState = EMonsterState::Dead;
	ClearTarget();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			AttackCooldownTimerHandle);
	}

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

	// 시체가 플레이어의 이동을 막지 않도록 함
	if (AActor* Owner = GetOwner())
	{
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		Owner->GetComponents<UPrimitiveComponent>(
			PrimitiveComponents);

		for (UPrimitiveComponent* Component :
			PrimitiveComponents)
		{
			if (!IsValid(Component))
			{
				continue;
			}

			Component->SetCollisionResponseToChannel(
				ECC_Pawn,
				ECR_Ignore);
		}
	}

	CalculateDrops();
	DropItems();

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
	if (!ItemPickupClass || !GetOwner()) return;

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

		const FVector DropLocation =
			GetOwner()->GetActorLocation() +
			FVector(
				FMath::RandRange(-DropRange, DropRange),
				FMath::RandRange(-DropRange, DropRange),
				DropHeight
			);

		APickupItem* Pickup = World->SpawnActor<APickupItem>(
			ItemPickupClass,
			DropLocation,
			FRotator::ZeroRotator
		);

		if (Pickup)
		{
			FItemInstance DroppedItem;
			DroppedItem.ItemData = Drop.Key;
			DroppedItem.Quantity = Drop.Value;

			Pickup->InitializePickup(DroppedItem);
		}
	}

	DropItem.Reset();
}

bool UMonsterComponent::IsInAttackRange()
{
	if (!IsValid(TargetActor) || !IsValid(GetOwner()))
	{
		return false;
	}

	const float Distance = GetDistanceToTarget();

	return Distance >= 0.0f && Distance <= AttackRange;
}

bool UMonsterComponent::CanAttack()
{
	if (bIsDead || !bCanAttack)
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
	if (!CanAttack() || !IsValid(AttackMontage))
	{
		return false;
	}

	USkeletalMeshComponent* Mesh =
		GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance =
		IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance)) return false;

	const FName SelectedSection = SelectAttackSection();
	if (SelectedSection.IsNone()) return false;

	const EMonsterState PreviousState = MonsterState;

	bCanAttack = false;
	SetMonsterState(EMonsterState::Attack);

	const float PlayedLength =
		AnimInstance->Montage_Play(AttackMontage);


	if (PlayedLength <= 0.0f)
	{
		// 재생 실패 시 공격 잠금 복구
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
		AttackMontage);

	AnimInstance->Montage_JumpToSection(
		SelectedSection, AttackMontage);

	// 재생에 성공한 경우에만 직전 공격으로 기록
	LastAttackSection = SelectedSection;

	return true;
}

void UMonsterComponent::FinishAttack()
{
	DisableAllAttackHitboxes();

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
	if (bIsDead || !IsValid(HitTarget) || !IsValid(StatComponent))
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

	const float Defense = FMath::Max(0.0f, TargetStat->GetDefensePower());

	const float DefenseMultiplier = DefenseScalingConstant / (DefenseScalingConstant + Defense);

	const float FinalDamage = FMath::Max(1.0f, 
		BaseDamage *
		FMath::Max(0.0f, AttackMultiplier) *
		DefenseMultiplier);

	UGameplayStatics::ApplyDamage(
		HitTarget,
		FinalDamage,
		GetOwner()->GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass());
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
	if (bIsDead || MonsterState != EMonsterState::Attack)
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
				if (AnimInstance->Montage_IsPlaying(AttackMontage))
				{
					AnimInstance->Montage_Stop(0.15f, AttackMontage);
				}
			}
		}
	}
}

void UMonsterComponent::PlayHitReaction()
{
	if (bIsDead)
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
	if (bIsDead)
	{
		return;
	}

	CancelAttack();
	PlayReactionMontage(ParriedMontage);
}

FName UMonsterComponent::SelectAttackSection() const
{
	if (!IsValid(AttackMontage))
	{
		return NAME_None;
	}

	TArray<FName> Candidates;

	for (int32 Index = 0; Index < AttackMontage->GetNumSections(); Index++)
	{
		const FName SectionName = AttackMontage->GetSectionName(Index);

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
			Index < AttackMontage->GetNumSections();
			++Index)
		{
			const FName SectionName =
				AttackMontage->GetSectionName(Index);

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
	if (bIsDead ||
		MonsterState != EMonsterState::Attack ||
		!IsValid(ActiveAttackHitbox))
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!IsValid(Player))
	{
		return;
	}

	UStatComponent* PlayerStat = IStatComponentUserInterface::Execute_GetStatComponent(Player);

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
	if (Montage != AttackMontage) return;

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

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UMonsterComponent::OnReactionMontageEnded);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		Montage);
}

void UMonsterComponent::OnReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bIsDead)
	{
		return;
	}

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
		FinishDeath();
		return;
	}

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance))
	{
		FinishDeath();
		return;
	}

	const float PlayedLength = AnimInstance->Montage_Play(DeathMontage);

	if (PlayedLength <= 0.0f)
	{
		FinishDeath();
		return;
	}

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

	FinishDeath();
}

void UMonsterComponent::FinishDeath()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	if (CorpseLifeTime <= 0.0f)
	{
		// 오브젝트 풀 리턴
		return;
	}

	Owner->SetLifeSpan(CorpseLifeTime);
}
