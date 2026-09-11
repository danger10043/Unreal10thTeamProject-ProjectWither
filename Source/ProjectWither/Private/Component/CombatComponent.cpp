#include "Component/CombatComponent.h"

#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/WeaponDataAsset.h"
#include "Interface/EnemyInterface.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Component/WeaponComponent.h"
#include "Component/MonsterComponent.h"
#include "Component/PlayerCameraComponent.h"
#include "Component/InventoryComponent.h"
#include "Engine/GameInstance.h"
#include "Framework/SubSystem/SavePointSubsystem.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/RootMotionSource.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
	const FName RollRootMotionSourceName(TEXT("CombatRoll"));
}

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<APlayerCharacter>(GetOwner());

	if (!ensureMsgf(
		IsValid(OwnerPlayer),
		TEXT("CombatComponent의 PlayerCharacter가 유효하지 않습니다. - 현재 OwnerPlayer : %s"),
		*GetNameSafe(GetOwner())
	))
	{
		return;
	}

	StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwnerPlayer);

	if (!ensureMsgf(
		IsValid(StatComponent),
		TEXT("CombatComponent의 Owner의 StatComponent가 유효하지 않습니다.")
	))
	{
		return;
	}

	WeaponComponent = IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwnerPlayer);

	if (!ensureMsgf(
		IsValid(WeaponComponent),
		TEXT("CombatComponent의 Owner의 WeaponComponent가 유효하지 않습니다.")
	))
	{
		return;
	}

	StatComponent->OnHealthZero.AddUniqueDynamic(
		this,
		&UCombatComponent::Die
	);
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FinishSwordApproach(false);

	EndSwordDamageWindow();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryTimerHandle);
		World->GetTimerManager().ClearTimer(ParryCooldownTimerHandle);
		World->GetTimerManager().ClearTimer(BlockStaminaTimerHandle);
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthZero.RemoveDynamic(
			this,
			&UCombatComponent::Die
		);
	}

	Super::EndPlay(EndPlayReason);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bSwordApproaching)
	{
		return;
	}

	if (!IsOwnerAlive() ||
		ActionState != EPlayerActionState::AttackingWithSword)
	{
		FinishSwordApproach(false);
		return;
	}

	if (!IsValid(WeaponComponent) ||
		!WeaponComponent->IsSwordEquipped() ||
		!IsAttackAssistTarget(AttackApproachTarget.Get()))
	{
		FinishSwordApproach(true);
		return;
	}

	const FVector Start = OwnerPlayer->GetActorLocation();
	const FVector Next = FMath::VInterpConstantTo(
		Start,
		AttackApproachDestination,
		DeltaTime,
		FMath::Max(1.0f, AttackApproachSpeed)
	);

	if (!IsAttackApproachPathClear(Start, Next))
	{
		FinishSwordApproach(true);
		return;
	}

	FHitResult MoveHit;
	OwnerPlayer->SetActorLocation(Next, true, &MoveHit);

	if (MoveHit.bBlockingHit ||
		FVector::DistSquared(
			OwnerPlayer->GetActorLocation(),
			AttackApproachDestination
		) <= FMath::Square(5.0f))
	{
		FinishSwordApproach(true);
	}
}

void UCombatComponent::Attack()
{
	switch (ResolveWeaponType())
	{
	case ECombatWeaponType::Sword:
		SwordAttack();
		break;

	case ECombatWeaponType::Gun:
		GunAttack();
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Attack - 장착된 무기가 없어 공격할 수 없습니다."));
		break;
	}
}

void UCombatComponent::SwordAttack()
{
	const bool bHasCurrentCombo =
		SwordComboSections.IsValidIndex(CurrentComboIndex) &&
		IsCurrentComboSection(SwordComboSections[CurrentComboIndex]);

	if (bHasCurrentCombo &&
		bNextAttackWindowOpen &&
		SwordComboSections.IsValidIndex(CurrentComboIndex + 1))
	{
		bNextAttackQueued = true;
		TryAdvanceSwordCombo();
		return;
	}

	if (ActionState == EPlayerActionState::AttackingWithSword)
	{
		return;
	}

	CancelSwordRecovery();

	StartAttack(
		ECombatWeaponType::Sword,
		EPlayerActionState::AttackingWithSword,
		SwordAttackStaminaCost
	);
}

void UCombatComponent::BeginNextAttackWindow(FName SectionName)
{
	if (!IsCurrentComboSection(SectionName))
	{
		return;
	}

	bNextAttackWindowOpen = true;
}

void UCombatComponent::EndNextAttackWindow(FName SectionName)
{
	if (!IsCurrentComboSection(SectionName))
	{
		return;
	}

	bNextAttackWindowOpen = false;
}

void UCombatComponent::ReachAttackCheckpoint(FName SectionName)
{
	if (!IsCurrentComboSection(SectionName))
	{
		return;
	}

	const int32 CheckpointComboIndex = CurrentComboIndex;
	const uint64 ExecutionId = SwordAttackExecutionId;

	bAttackCheckpointReached = true;
	TryAdvanceSwordCombo();

	// 예약된 다음 공격이 시작되었거나 현재 공격이 중단되었다면 새 공격에 후딜 적용 안함
	if (ExecutionId != SwordAttackExecutionId ||
		CurrentComboIndex != CheckpointComboIndex ||
		!IsCurrentComboSection(SectionName))
	{
		return;
	}

	EndSwordDamageWindow();

	bSwordRecovery = true;
	OwnerPlayer->SetCanMove(true);
	SetActionState(EPlayerActionState::None);
}

void UCombatComponent::CancelSwordRecovery()
{
	if (!bSwordRecovery)
	{
		return;
	}

	++SwordAttackExecutionId;

	ResetSwordCombo();
	EndSwordDamageWindow();

	USkeletalMeshComponent* Mesh = IsValid(OwnerPlayer)
		? OwnerPlayer->GetMesh()
		: nullptr;

	UAnimInstance* AnimInstance = IsValid(Mesh)
		? Mesh->GetAnimInstance()
		: nullptr;

	if (IsValid(AnimInstance) && IsValid(SwordAttackMontage))
	{
		AnimInstance->Montage_Stop(0.2f, SwordAttackMontage);
	}
}

void UCombatComponent::BeginSwordDamageWindow()
{
	if (bSwordApproaching)
	{
		return;
	}

	EndSwordDamageWindow();

	if (ActionState != EPlayerActionState::AttackingWithSword
		|| !IsValid(WeaponComponent)
		|| !WeaponComponent->IsSwordEquipped()) return;

	WeaponComponent->BeginSwordTrail();

	UCapsuleComponent* SwordCollision = FindSwordCollision();

	if (!IsValid(SwordCollision))
	{
		UE_LOG(LogTemp, Warning, TEXT("검 Actor에서 SwordHitCollision Capsule 을 찾지 못했습니다."));
		return;
	}

	SwordHitActors.Reset();
	ActiveSwordCollision = SwordCollision;

	ActiveSwordCollision->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UCombatComponent::HandleSwordCollisionBeginOverlap
	);
	ActiveSwordCollision->SetGenerateOverlapEvents(true);
	ActiveSwordCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UCombatComponent::EndSwordDamageWindow()
{
	if (IsValid(WeaponComponent))
	{
		WeaponComponent->EndSwordTrail();
	}

	if (IsValid(ActiveSwordCollision))
	{
		ActiveSwordCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ActiveSwordCollision->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&UCombatComponent::HandleSwordCollisionBeginOverlap
		);
	}

	ActiveSwordCollision = nullptr;
	SwordHitActors.Reset();
}

void UCombatComponent::GunAttack()
{
	if (!CanAttack())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - 현재 공격할 수 없는 상태입니다.")
		);
		return;
	}

	if (!IsValid(WeaponComponent) || !WeaponComponent->IsGunEquipped())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - 총이 장착되어 있지 않습니다.")
		);
		return;
	}

	const UPlayerCameraComponent* Camera =
		IsValid(OwnerPlayer)
		? OwnerPlayer->FindComponentByClass<UPlayerCameraComponent>()
		: nullptr;

	if (!IsValid(Camera) || !Camera->IsZooming())
	{
		return;
	}

	if (!TrySpendStamina(GunAttackStaminaCost))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - 스태미나가 부족합니다.")
		);
		return;
	}

	if (!WeaponComponent->FireGun())
	{
		StatComponent->RecoverStamina(GunAttackStaminaCost);

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - 총기 발사에 실패했습니다.")
		);
		return;
	}

	if (ActionState == EPlayerActionState::Reload)
	{
		return;
	}

	if (!IsValid(OwnerPlayer) || !IsValid(GunAttackMontage))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - OwnerPlayer 또는 GunAttackMontage가 유효하지 않습니다.")
		);
		return;
	}

	const float PlayedLength = OwnerPlayer->PlayAnimMontage(GunAttackMontage);

	if (PlayedLength <= 0.0f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::GunAttack - 발사 몽타주 재생에 실패했습니다.")
		);
	}
}

void UCombatComponent::Roll()
{
	if (!CanRoll())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Roll - 현재 Roll을 실행할 수 없습니다."));
		return;
	}

	UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement();
	UAnimInstance* AnimInstance = OwnerPlayer->GetMesh() ? OwnerPlayer->GetMesh()->GetAnimInstance() : nullptr;

	if (!IsValid(Movement))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Roll - Movement Component가 유효하지 않습니다."));
		return;
	}
	if (!IsValid(AnimInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Roll - AnimInstance가 유효하지 않습니다."));
		return;
	}
	if (!TrySpendStamina(RollStaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::Roll - 스태미나가 충분하지 않습니다."));
		return;
	}

	// 입력 중 - 입력 방향으로 구르기
	FVector RollDirection = OwnerPlayer->GetLastMovementInputVector().GetSafeNormal2D();

	// 입력 중 X - 캐릭터가 바라보는 방향으로 구르기
	if (RollDirection.IsNearlyZero())
	{
		RollDirection = OwnerPlayer->GetActorForwardVector().GetSafeNormal2D();
	}

	OwnerPlayer->SetActorRotation(RollDirection.Rotation());
	OwnerPlayer->SetCanMove(false);
	SetActionState(EPlayerActionState::Rolling);

	const float PlayedLength = OwnerPlayer->PlayAnimMontage(RollMontage);

	if (PlayedLength <= 0.0f)
	{
		StatComponent->RecoverStamina(RollStaminaCost);
		OwnerPlayer->SetCanMove(true);
		FinishAction(EPlayerActionState::Rolling);
		return;
	}

	const float RollSpeed = PlayedLength > UE_SMALL_NUMBER ? RollDistance / PlayedLength : 0.0f;

	TSharedPtr<FRootMotionSource_ConstantForce> RollMovement = MakeShared<FRootMotionSource_ConstantForce>();

	RollMovement->InstanceName = RollRootMotionSourceName;
	RollMovement->Priority = 500;
	RollMovement->AccumulateMode = ERootMotionAccumulateMode::Override;
	RollMovement->Duration = PlayedLength;
	RollMovement->Force = RollDirection * RollSpeed;
	RollMovement->StrengthOverTime = RollSpeedCurve;
	RollMovement->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	RollMovement->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

	Movement->ApplyRootMotionSource(RollMovement);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UCombatComponent::OnRollMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);

	OnRollStarted();
}

void UCombatComponent::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != RollMontage) return;
	if (ActionState != EPlayerActionState::Rolling) return;

	if (IsValid(OwnerPlayer))
	{
		if (UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement())
		{
			Movement->RemoveRootMotionSource(RollRootMotionSourceName);
		}
	}

	if (IsValid(OwnerPlayer))
	{
		OwnerPlayer->SetCanMove(true);
	}
	FinishAction(EPlayerActionState::Rolling);
}

void UCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted, uint64 ExecutionId)
{
	if (Montage != SwordAttackMontage || ExecutionId != SwordAttackExecutionId) { return; }

	// NotifyEnd가 호출되지 않고 몽타주가 종료됨을 대비
	EndSwordDamageWindow();
	ResetSwordCombo();

	if (ActionState != EPlayerActionState::AttackingWithSword) { return; }

	if (IsValid(OwnerPlayer))
	{
		OwnerPlayer->SetCanMove(true);
	}

	FinishAction(EPlayerActionState::AttackingWithSword);
}

void UCombatComponent::HandleSwordCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (ActionState != EPlayerActionState::AttackingWithSword ||
		!IsValid(OtherActor) ||
		OtherActor == OwnerPlayer ||
		SwordHitActors.Contains(OtherActor)) return;

	if (!OtherActor->GetClass()->ImplementsInterface(UEnemyInterface::StaticClass())) return;

	const float SwordDamage = CalculateSwordDamage();

	if (SwordDamage <= 0.0f) return;

	SwordHitActors.Add(OtherActor);
	
	const UWeaponDataAsset* WeaponData =
		IsValid(WeaponComponent)
		? WeaponComponent->GetCurrentWeaponData()
		: nullptr;

	UNiagaraSystem* HitEffect =
		IsValid(WeaponData) ? WeaponData->GetWeaponHitEffect() : nullptr;

	FVector EffectLocation = IsValid(OverlappedComponent)
		? OverlappedComponent->GetComponentLocation()
		: OtherActor->GetActorLocation();

	if (bFromSweep && !SweepResult.bStartPenetrating)
	{
		EffectLocation = SweepResult.ImpactPoint;
	}
	else if (IsValid(OtherComponent))
	{
		FVector ClosestPoint;
		const float Distance = OtherComponent->GetClosestPointOnCollision(
			EffectLocation,
			ClosestPoint
		);

		if (Distance > 0.0f)
		{
			EffectLocation = ClosestPoint;
		}
	}

	const float AppliedDamage = UGameplayStatics::ApplyDamage(
		OtherActor,
		SwordDamage,
		IsValid(OwnerPlayer) ? OwnerPlayer->GetController() : nullptr,
		OwnerPlayer,
		UDamageType::StaticClass()
	);

	if (AppliedDamage > 0.0f && IsValid(HitEffect))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			HitEffect,
			EffectLocation,
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true
		);
	}
}

UCapsuleComponent* UCombatComponent::FindSwordCollision() const
{
	if (!IsValid(WeaponComponent)) return nullptr;

	AActor* CurrentWeaponActor = WeaponComponent->GetWeaponActor();

	if (!IsValid(CurrentWeaponActor)) return nullptr;

	TArray<UCapsuleComponent*> CapsuleComponents;
	CurrentWeaponActor->GetComponents<UCapsuleComponent>(CapsuleComponents);

	for (UCapsuleComponent* CapsuleComponent : CapsuleComponents)
	{
		if (IsValid(CapsuleComponent) && CapsuleComponent->ComponentHasTag(TEXT("SwordHitCollision")))
		{
			return CapsuleComponent;
		}
	}

	return nullptr;
}

float UCombatComponent::CalculateSwordDamage() const
{
	if (!IsValid(StatComponent) || !IsValid(WeaponComponent)) { return 0.0f; }

	// 데이터 에셋뿐 아니라 강화 단계가 들어 있는 아이템 인스턴스를 가져온다.
	const FItemInstance* CurrentWeapon = WeaponComponent->GetCurrentWeapon();

	if (CurrentWeapon == nullptr || !IsValid(CurrentWeapon->ItemData) || CurrentWeapon->Quantity <= 0) { return 0.0f; }

	const UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(CurrentWeapon->ItemData.Get());

	if (!IsValid(WeaponData) || WeaponData->GetWeaponType() != EWeaponType::Sword) { return 0.0f; }

	const float MinAttackPower = FMath::Min(StatComponent->GetMinAttackPower(), StatComponent->GetMaxAttackPower());
	
	const float MaxAttackPower = FMath::Max(StatComponent->GetMinAttackPower(), StatComponent->GetMaxAttackPower());

	const float CharacterAttackPower = FMath::FRandRange(MinAttackPower, MaxAttackPower);

	const float EnhancedWeaponPower = WeaponData->GetEnhancedWeaponPower(CurrentWeapon->EnhanceLevel);

	return FMath::Max(0.0f, CharacterAttackPower + EnhancedWeaponPower);
}

void UCombatComponent::StartBlock()
{
	if (!CanBlock()) { return; }

	OwnerPlayer->SetCanMove(false);

	SetActionState(EPlayerActionState::Blocking);
	OpenParryWindow();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().SetTimer(
			BlockStaminaTimerHandle,
			this,
			&UCombatComponent::ConsumeBlockStamina,
			BlockHoldStaminaInterval,
			true
		);
	}

	// 가드 애니메이션 시작
}

void UCombatComponent::StopBlock()
{
	if (ActionState != EPlayerActionState::Blocking) { return; }

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BlockStaminaTimerHandle);
	}

	CloseParryWindow();
	FinishAction(EPlayerActionState::Blocking);

	if (IsValid(OwnerPlayer))
	{
		OwnerPlayer->SetCanMove(true);
	}

	// 가드 애니메이션 종료
}

float UCombatComponent::ReceiveHit(float DamageAmount, AActor* DamageCauser, AController* EventInstigator)
{
	if (!IsOwnerAlive() || DamageAmount <= 0.0f) { return 0.0f; }

	if (ActionState == EPlayerActionState::Rolling)
	{
		return 0.0f;
	}

	if (ActionState == EPlayerActionState::Blocking)
	{
		const bool bWasParry = bParryWindowOpen;

		if (TrySpendStamina(BlockStaminaCost))
		{
			if (bWasParry)
			{
				CloseParryWindow();

				if (IsValid(OwnerPlayer) && IsValid(ParryMontage))
				{
					const float PlayedLength = OwnerPlayer->PlayAnimMontage(ParryMontage);

					if (PlayedLength <= 0.0f)
					{
						UE_LOG(
							LogTemp,
							Warning,
							TEXT("UCombatComponent::ReceiveHit - 패링 몽타주 재생에 실패했습니다.")
						);
					}
				}

				OnParrySucceeded();

				// 공격한 적에게 패링 성공 전달
				if (IsValid(DamageCauser))
				{
					UMonsterComponent* MonsterComponent = DamageCauser->FindComponentByClass<UMonsterComponent>();

					if (IsValid(MonsterComponent))
					{
						MonsterComponent->HandleParried();
					}
				}
			}
			else
			{
				OnBlockSucceeded();
			}

			// 공격은 막았지만 다음 공격 비용이 부족하면 가드 해제
			if (!StatComponent->HasEnoughStamina(BlockStaminaCost))
			{
				StopBlock();
			}

			if (bWasParry)
			{
				return 0.0f;
			}

			return StatComponent->ApplyDamage(DamageAmount, 0.5f);
		}

		// 공격을 막을 비용이 부족하므로 가드 실패
		StopBlock();
	}

	const float AppliedDamage =
		StatComponent->ApplyDamage(DamageAmount);

	if (AppliedDamage <= 0.0f)
	{
		return 0.0f;
	}

	// 대미지를 받고도 살아 있다면 피격 반응
	if (IsOwnerAlive())
	{
		StartHitReaction();
		OnHitReceived();
	}

	return AppliedDamage;
}

void UCombatComponent::Die()
{
	if (ActionState == EPlayerActionState::Dead) return;

	if (!IsValid(OwnerPlayer)) return;

	UAnimInstance* AnimInstance = OwnerPlayer->GetMesh() ? OwnerPlayer->GetMesh()->GetAnimInstance() : nullptr;

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(ParryTimerHandle);
		TimerManager.ClearTimer(ParryCooldownTimerHandle);
		TimerManager.ClearTimer(BlockStaminaTimerHandle);
	}

	bParryWindowOpen = false;
	bParryOnCooldown = false;

	EndSwordDamageWindow();

	if (UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement())
	{
		Movement->RemoveRootMotionSource(RollRootMotionSourceName);
		Movement->StopMovementImmediately();
	}

	// 기존 몽타주 종료 롤백이 이동이나 상태를 복구하지 못하도록 막기
	OwnerPlayer->SetCanMove(false);
	SetActionState(EPlayerActionState::Dead);

	UInventoryComponent* Inventory =
		OwnerPlayer->FindComponentByClass<UInventoryComponent>();

	if (IsValid(Inventory))
	{
		const int32 CurrentGold = FMath::Max(0, Inventory->GetGold());
		const int32 LostGold = static_cast<int32>(
			static_cast<int64>(CurrentGold) * 70 / 100
			);

		if (LostGold > 0)
		{
			Inventory->SpendGold(LostGold);
		}
	}

	if (!IsValid(AnimInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Die - AnimInstance가 유효하지 않습니다."));
		ScheduleRespawn();
		return;
	}

	// 다른 모든 몽타주 즉시 종료하고 사망 몽타주 재생하기
	AnimInstance->StopAllMontages(0.0f);
	if (!IsValid(DeathMontage))
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Die - Death Montage가 유효하지 않습니다."));
		ScheduleRespawn();
		return;
	}

	const float PlayedLength = OwnerPlayer->PlayAnimMontage(DeathMontage);
	if (PlayedLength <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent::Die - Death Montage 재생에 실패했습니다."));
		ScheduleRespawn();
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UCombatComponent::OnDeathMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);

	// 사망 몽타주 마지막 프레임을 홀드/루프하도록 만들어서 Montage_SetEndDelegate가
	// 끝내 호출되지 않는 경우를 대비한 백업 타이머. 델리게이트가 먼저 호출되면
	// HandleRespawn의 ActionState 검사로 인해 이 타이머는 아무 효과 없이 무시된다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&UCombatComponent::HandleRespawn,
			PlayedLength,
			false
		);
	}
}

void UCombatComponent::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DeathMontage) return;
	if (ActionState != EPlayerActionState::Dead) return;

	HandleRespawn();
}

void UCombatComponent::ScheduleRespawn()
{
	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		HandleRespawn();
		return;
	}

	// Die()는 StatComponent::ApplyDamage() 안에서 OnHealthZero 델리게이트로 동기 호출된다.
	// 여기서 바로 HandleRespawn()을 부르면 ApplyDamage()가 아직 실행 중인 상태에서
	// 체력/행동 상태가 되돌아가 버려서, 뒤이은 피격 처리(히트 리액션 등)가 방금 되돌린
	// 상태를 다시 덮어쓰는 문제가 생긴다. 다음 틱으로 미뤄서 호출 스택을 완전히 빠져나온 뒤 부활시킨다.
	World->GetTimerManager().SetTimerForNextTick(this, &UCombatComponent::HandleRespawn);
}

void UCombatComponent::HandleRespawn()
{
	if (!IsValid(OwnerPlayer)) return;

	// 몽타주 종료 델리게이트와 백업 타이머 중 먼저 호출된 쪽만 실제로 부활을 처리하도록 방지
	if (ActionState != EPlayerActionState::Dead) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}

	// 사망 몽타주가 자연 종료되지 않고 마지막 프레임을 계속 재생 중일 수 있으므로
	// 로코모션으로 되돌아갈 수 있도록 명시적으로 멈춘다.
	if (UAnimInstance* AnimInstance = OwnerPlayer->GetMesh() ? OwnerPlayer->GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->StopAllMontages(0.0f);
	}

	FTransform RespawnTransform;
	bool bHasRespawnTransform = false;

	if (UGameInstance* GameInstance = OwnerPlayer->GetGameInstance())
	{
		if (USavePointSubsystem* SavePointSubsystem = GameInstance->GetSubsystem<USavePointSubsystem>())
		{
			if (SavePointSubsystem->HasCurrentSavePoint())
			{
				RespawnTransform = SavePointSubsystem->GetCurrentSavePointTransform();
				bHasRespawnTransform = true;
			}
		}
	}

	// 활성화된 세이브 포인트가 아직 없으면 죽은 자리 대신 레벨의 PlayerStart로 부활한다.
	if (!bHasRespawnTransform)
	{
		if (AActor* PlayerStart = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()))
		{
			RespawnTransform = PlayerStart->GetActorTransform();
			bHasRespawnTransform = true;
		}
	}

	if (bHasRespawnTransform)
	{
		if (UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}

		OwnerPlayer->SetActorLocationAndRotation(
			RespawnTransform.GetLocation(),
			RespawnTransform.GetRotation(),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}
	// PlayerStart조차 없으면 최후의 수단으로 죽은 자리에서 그대로 부활한다.

	if (IsValid(StatComponent))
	{
		StatComponent->ResetStat();
	}

	OwnerPlayer->SetCanMove(true);
	SetActionState(EPlayerActionState::None);
}

void UCombatComponent::FinishAction(EPlayerActionState ExpectedState)
{
	if (ActionState != ExpectedState) return;

	SetActionState(EPlayerActionState::None);
}

bool UCombatComponent::CanReload() const
{
	if (!IsOwnerAlive()) return false;

	if (ActionState != EPlayerActionState::None) return false;

	if (!OwnerPlayer->CanMove()) return false;

	if (!IsValid(WeaponComponent) || !WeaponComponent->IsGunEquipped())
	{
		return false;
	}

	if (!IsValid(ReloadMontage)) return false;

	const UAnimInstance* AnimInstance =
		OwnerPlayer->GetMesh()
		? OwnerPlayer->GetMesh()->GetAnimInstance()
		: nullptr;

	return IsValid(AnimInstance);
}

void UCombatComponent::PlayReloadMontage()
{
	if (!CanReload()) return;

	UAnimInstance* AnimInstance = OwnerPlayer->GetMesh()->GetAnimInstance();
	const uint64 ExecutionId = ++ReloadExecutionId;

	SetActionState(EPlayerActionState::Reload);

	// 상태 변경 이벤트에서 피격이나 사망 등으로 전환된 경우.
	if (ActionState != EPlayerActionState::Reload) return;

	const float PlayedLength = AnimInstance->Montage_Play(ReloadMontage);

	if (PlayedLength <= 0.0f)
	{
		FinishAction(EPlayerActionState::Reload);
		UE_LOG(LogTemp, Warning, TEXT("재장전 몽타주 재생에 실패했습니다."));
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UCombatComponent::OnReloadMontageEnded,
		ExecutionId
	);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ReloadMontage);
}

void UCombatComponent::OnReloadMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted,
	uint64 ExecutionId
)
{
	if (Montage != ReloadMontage || ExecutionId != ReloadExecutionId)
	{
		return;
	}

	// 정상 종료와 중단 모두 처리하되, 피격/사망 상태는 덮어쓰지 않는다.
	FinishAction(EPlayerActionState::Reload);
}

bool UCombatComponent::CanAttack() const
{
	if (!IsOwnerAlive()) { return false; }

	if (!IsValid(OwnerPlayer) || !OwnerPlayer->CanMove()) { return false; }

	if (ActionState != EPlayerActionState::None) { return false; }

	return true;
}

bool UCombatComponent::CanRoll() const
{
	if (!IsOwnerAlive())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: OwnerPlayer 또는 StatComponent가 유효하지 않거나 사망 상태입니다."));
		return false;
	}

	if (!IsValid(RollMontage))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: CombatComponent에 RollMontage가 지정되지 않았습니다."));
		return false;
	}

	if (!IsValid(RollSpeedCurve))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: CombatComponent에 RollSpeedCurve가 지정되지 않았습니다."));
		return false;
	}

	if (!OwnerPlayer->CanMove())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: PlayerCharacter의 bCanMove가 false입니다."));
		return false;
	}

	if (ActionState != EPlayerActionState::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: 현재 ActionState는 %d입니다."), static_cast<int32>(ActionState));
		return false;
	}

	if (!StatComponent->HasEnoughStamina(RollStaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: 스태미나가 부족합니다. 현재 %.1f / 필요 %.1f"), StatComponent->GetCurrentStamina(), RollStaminaCost);
		return false;
	}

	const UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement();

	if (!IsValid(Movement))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: CharacterMovement가 유효하지 않습니다."));
		return false;
	}

	if (!Movement->IsMovingOnGround())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: 캐릭터가 지상에 있지 않습니다."));
		return false;
	}

	return true;
}

bool UCombatComponent::CanBlock() const
{
	if (!IsOwnerAlive()) { return false; }

	if (!IsValid(OwnerPlayer) || !OwnerPlayer->CanMove()) { return false; }

	if (ActionState != EPlayerActionState::None) { return false; }

	if (!IsValid(WeaponComponent) || !WeaponComponent->IsSwordEquipped()) { return false; }

	if (!IsValid(StatComponent) || !StatComponent->HasEnoughStamina(BlockStaminaCost)) { return false; }

	return true;
}

ECombatWeaponType UCombatComponent::ResolveWeaponType_Implementation() const
{
	if (!IsValid(WeaponComponent))
	{
		return ECombatWeaponType::None;
	}

	switch (WeaponComponent->GetWeaponType())
	{
	case EWeaponType::Sword:
		return ECombatWeaponType::Sword;

	case EWeaponType::Gun:
		return ECombatWeaponType::Gun;

	default:
		return ECombatWeaponType::None;
	}
}

bool UCombatComponent::IsOwnerAlive() const
{
	if (!IsValid(OwnerPlayer)) {
		UE_LOG(LogTemp, Warning, TEXT("OwnerPlayer 가 유효하지 않습니다. 현재 OwnerPlayer - %s"),
			*GetNameSafe(GetOwner()));
		return false;
	}
	if (!IsValid(StatComponent)) {
		UE_LOG(LogTemp, Warning, TEXT("StatComponent 가 유효하지 않습니다."));
		return false;
	}
	if (StatComponent->IsHealthZero()) {
		UE_LOG(LogTemp, Warning, TEXT("플레이어의 체력이 0입니다."));
		return false;
	}
	if (ActionState == EPlayerActionState::Dead) {
		UE_LOG(LogTemp, Warning, TEXT("플레이어는 현재 사망 상태입니다."));
		return false;
	}
	return true;
}

bool UCombatComponent::TrySpendStamina(float Cost)
{
	if (Cost <= 0.0f) return true;
	if (!IsValid(StatComponent)) return false;

	return StatComponent->UseStamina(Cost) >= Cost;
}

void UCombatComponent::StartAttack(ECombatWeaponType RequiredWeapon, EPlayerActionState AttackState, float StaminaCost)
{
	if (!CanAttack()) { return; }

	if (RequiredWeapon != ECombatWeaponType::Sword || !IsValid(SwordAttackMontage)) { return; }

	UAnimInstance* AnimInstance = OwnerPlayer->GetMesh() ? OwnerPlayer->GetMesh()->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance)) { return; }

	if (SwordComboSections.IsEmpty())
	{
		return;
	}

	TSet<FName> CheckedSections;
	for (const FName SectionName : SwordComboSections)
	{
		if (SectionName.IsNone() ||
			CheckedSections.Contains(SectionName) ||
			SwordAttackMontage->GetSectionIndex(SectionName) == INDEX_NONE)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("CombatComponent::StartAttack - 검 콤보 섹션 설정 오류 : %s"),
				*SectionName.ToString()
			);
			return;
		}
		CheckedSections.Add(SectionName);
	}

	if (!IsValid(StatComponent) || !StatComponent->HasEnoughStamina(StaminaCost))
	{
		return;
	}

	if (!TrySpendStamina(StaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("공격에 필요한 스태미나가 부족합니다."));
		return;
	}

	const uint64 ExecutionId = ++SwordAttackExecutionId;

	// SetCanMove(false) 내부에서 달리기도 종료됩니다.
	OwnerPlayer->SetCanMove(false);
	SetActionState(AttackState);

	ResetSwordCombo();
	CurrentComboIndex = 0;

	const float PlayedLength = OwnerPlayer->PlayAnimMontage(
		SwordAttackMontage,
		1.0f,
		SwordComboSections[0]
	);

	if (PlayedLength <= 0.0f)
	{
		StatComponent->RecoverStamina(StaminaCost);
		OwnerPlayer->SetCanMove(true);
		FinishAction(AttackState);
		return;
	}

	FOnMontageEnded EndDelegate;

	EndDelegate.BindUObject(
		this,
		&UCombatComponent::OnAttackMontageEnded,
		ExecutionId
	);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, SwordAttackMontage);

	for (const FName SectionName : SwordComboSections)
	{
		AnimInstance->Montage_SetNextSection(
			SectionName,
			NAME_None,
			SwordAttackMontage
		);
	}

	OnAttackStarted(RequiredWeapon);

	BeginSwordApproach();
}

bool UCombatComponent::IsCurrentComboSection(FName SectionName) const
{

	const bool bCanContinueCombo =
		ActionState == EPlayerActionState::AttackingWithSword ||
		(ActionState == EPlayerActionState::None && bSwordRecovery);

	if (bSwordApproaching ||
		!bCanContinueCombo ||
		!IsOwnerAlive() ||
		!IsValid(WeaponComponent) ||
		!WeaponComponent->IsSwordEquipped() ||
		!IsValid(SwordAttackMontage) ||
		!SwordComboSections.IsValidIndex(CurrentComboIndex) ||
		SwordComboSections[CurrentComboIndex] != SectionName)
	{
		return false;
	}

	USkeletalMeshComponent* Mesh = OwnerPlayer->GetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh)
		? Mesh->GetAnimInstance()
		: nullptr;

	return IsValid(AnimInstance) &&
		AnimInstance->Montage_IsPlaying(SwordAttackMontage) &&
		AnimInstance->Montage_GetCurrentSection(SwordAttackMontage) == SectionName;
}

void UCombatComponent::TryAdvanceSwordCombo()
{
	if (!bNextAttackQueued ||
		!bAttackCheckpointReached ||
		!SwordComboSections.IsValidIndex(CurrentComboIndex) ||
		!SwordComboSections.IsValidIndex(CurrentComboIndex + 1))
	{
		return;
	}

	if (!IsCurrentComboSection(SwordComboSections[CurrentComboIndex]))
	{
		return;
	}

	if (!IsValid(StatComponent) || !StatComponent->HasEnoughStamina(SwordAttackStaminaCost))
	{
		bNextAttackQueued = false;
		return;
	}

	UAnimInstance* AnimInstance = OwnerPlayer->GetMesh()->GetAnimInstance();

	if (!TrySpendStamina(SwordAttackStaminaCost))
	{
		bNextAttackQueued = false;
		return;
	}

	EndSwordDamageWindow();

	const int32 NextIndex = CurrentComboIndex + 1;
	ResetSwordCombo();
	CurrentComboIndex = NextIndex;

	OwnerPlayer->SetCanMove(false);
	SetActionState(EPlayerActionState::AttackingWithSword);

	AnimInstance->Montage_JumpToSection(
		SwordComboSections[CurrentComboIndex],
		SwordAttackMontage
	);

	BeginSwordApproach();
}

void UCombatComponent::ResetSwordCombo()
{
	CurrentComboIndex = INDEX_NONE;
	bNextAttackWindowOpen = false;
	bNextAttackQueued = false;
	bAttackCheckpointReached = false;
	bSwordRecovery = false;
}

bool UCombatComponent::IsAttackAssistTarget(AActor* Target) const
{
	if (!IsValid(OwnerPlayer) ||
		!IsValid(Target) ||
		Target == OwnerPlayer.Get() ||
		Target->IsHidden() ||
		!Target->GetActorEnableCollision() ||
		!Target->GetClass()->ImplementsInterface(UEnemyInterface::StaticClass()))
	{
		return false;
	}

	const UMonsterComponent* Monster =
		Target->FindComponentByClass<UMonsterComponent>();

	if (IsValid(Monster) && Monster->IsDead())
	{
		return false;
	}

	const FVector Difference =
		Target->GetActorLocation() - OwnerPlayer->GetActorLocation();

	return AttackSearchRadius > 0.0f &&
		Difference.SizeSquared() <= FMath::Square(AttackSearchRadius) &&
		FMath::Abs(Difference.Z) <= AttackTargetHeightTolerance;
}

AActor* UCombatComponent::FindAttackAssistTarget() const
{
	if (!IsValid(OwnerPlayer) || !GetWorld() || AttackSearchRadius <= 0.0f)
	{
		return nullptr;
	}

	const UPlayerCameraComponent* Camera =
		OwnerPlayer->FindComponentByClass<UPlayerCameraComponent>();

	if (IsValid(Camera))
	{
		AActor* LockedTarget = Camera->GetLockOnTarget();
		if (IsAttackAssistTarget(LockedTarget))
		{
			return LockedTarget;
		}
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPlayer.Get());

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		OwnerPlayer->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams::AllObjects,
		FCollisionShape::MakeSphere(AttackSearchRadius),
		Params
	);

	AActor* ClosestTarget = nullptr;
	double ClosetDistanceSquared = TNumericLimits<double>::Max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!IsAttackAssistTarget(Candidate))
		{
			continue;
		}

		const double DistanceSquared = FVector::DistSquared(
			OwnerPlayer->GetActorLocation(),
			Candidate->GetActorLocation()
		);

		if (DistanceSquared < ClosetDistanceSquared)
		{
			ClosetDistanceSquared = DistanceSquared;
			ClosestTarget = Candidate;
		}
	}

	return ClosestTarget;
}

bool UCombatComponent::IsAttackApproachPathClear(const FVector& Start, const FVector& End) const
{
	if (!IsValid(OwnerPlayer) || !GetWorld())
	{
		return false;
	}

	const UCapsuleComponent* Capsule = OwnerPlayer->GetCapsuleComponent();
	const UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement();

	if (!IsValid(Capsule) ||
		!IsValid(Movement) ||
		!OwnerPlayer->GetActorEnableCollision() ||
		!Capsule->IsQueryCollisionEnabled())
	{
		return false;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPlayer.Get());

	if (IsValid(WeaponComponent))
	{
		if (AActor* Weapon = WeaponComponent->GetWeaponActor())
		{
			Params.AddIgnoredActor(Weapon);
		}
	}

	const FCollisionResponseParams Responses(Capsule->GetCollisionResponseToChannels());

	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	FHitResult BodyHit;
	if (GetWorld()->SweepSingleByChannel(
		BodyHit,
		Start,
		End,
		Capsule->GetComponentQuat(),
		Capsule->GetCollisionObjectType(),
		FCollisionShape::MakeCapsule(Radius, HalfHeight),
		Params,
		Responses
	))
	{
		return false;
	}

	const int32 Samples = FMath::Max(1, FMath::CeilToInt(FVector::Distance(Start, End) / 20.0f));

	for (int32 Index = 0; Index <= Samples; ++Index)
	{
		const FVector Position = FMath::Lerp(
			Start,
			End,
			static_cast<float>(Index) / Samples
		);

		const FVector Feet = Position - FVector(0.0f, 0.0f, HalfHeight);
		FHitResult FloorHit;

		if (!GetWorld()->LineTraceSingleByChannel(
			FloorHit,
			Feet + FVector(0.0f, 0.0f, 10.0f),
			Feet - FVector(0.0f, 0.0f, 15.0f),
			Capsule->GetCollisionObjectType(),
			Params,
			Responses
		) || !Movement->IsWalkable(FloorHit))
		{
			return false;
		}
	}

	return true;
}

void UCombatComponent::BeginSwordApproach()
{
	if (!IsOwnerAlive() ||
		ActionState != EPlayerActionState::AttackingWithSword ||
		!IsValid(SwordAttackMontage))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = OwnerPlayer->GetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement();

	if (!IsValid(AnimInstance) ||
		!IsValid(Movement) ||
		!Movement->IsWalking() ||
		!AnimInstance->Montage_IsPlaying(SwordAttackMontage))
	{
		return;
	}

	AActor* Target = FindAttackAssistTarget();
	if (!IsValid(Target))
	{
		return;
	}

	const FVector Start = OwnerPlayer->GetActorLocation();
	const FVector ToTarget = Target->GetActorLocation() - Start;
	const FVector Direction = ToTarget.GetSafeNormal2D();

	if (Direction.IsNearlyZero())
	{
		return;
	}

	float TargetRadius = 0.0f;
	float TargetHalfHeight = 0.0f;
	Target->GetSimpleCollisionCylinder(TargetRadius, TargetHalfHeight);

	const float StopDistance =
		OwnerPlayer->GetCapsuleComponent()->GetScaledCapsuleRadius() +
		TargetRadius +
		FMath::Max(0.0f, AttackApproachGap);

	const float TravelDistance =
		FMath::Max(0.0f, static_cast<float>(ToTarget.Size2D()) - StopDistance);

	if (TravelDistance <= 5.0f)
	{
		OwnerPlayer->SetActorRotation(
			FRotator(0.0f, Direction.Rotation().Yaw, 0.0f)
		);
		return;
	}

	const FVector Destination = Start + Direction * TravelDistance;
	if (!IsAttackApproachPathClear(Start, Destination))
	{
		return;
	}
	EndSwordDamageWindow();

	AttackApproachTarget = Target;
	AttackApproachDestination = Destination;
	bSwordApproaching = true;

	AnimInstance->Montage_Pause(SwordAttackMontage);
	Movement->StopMovementImmediately();
	Movement->DisableMovement();

	SetComponentTickEnabled(true);
}

void UCombatComponent::FinishSwordApproach(bool bResumeAttack)
{
	if (!bSwordApproaching)
	{
		return;
	}

	bSwordApproaching = false;
	SetComponentTickEnabled(false);

	AActor* Target = AttackApproachTarget.Get();
	AttackApproachTarget.Reset();

	if (!IsValid(OwnerPlayer))
	{
		return;
	}

	UCharacterMovementComponent* Movement = OwnerPlayer->GetCharacterMovement();
	if (IsValid(Movement) && Movement->MovementMode == MOVE_None)
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	USkeletalMeshComponent* Mesh = OwnerPlayer->GetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance) || !IsValid(SwordAttackMontage))
	{
		return;
	}

	if (!bResumeAttack)
	{
		++SwordAttackExecutionId;
		AnimInstance->Montage_Stop(0.0f, SwordAttackMontage);
		return;
	}

	if (IsAttackAssistTarget(Target))
	{
		const FVector Direction =
			(Target->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal2D();

		if (!Direction.IsNearlyZero())
		{
			OwnerPlayer->SetActorRotation(
				FRotator(0.0f, Direction.Rotation().Yaw, 0.0f)
			);
		}
	}

	AnimInstance->Montage_Resume(SwordAttackMontage);
}

void UCombatComponent::OpenParryWindow()
{
	if (bParryOnCooldown) { return; }

	UWorld* World = GetWorld();
	if (!IsValid(World)) { return; }

	bParryWindowOpen = true;
	bParryOnCooldown = true;

	FTimerManager& TimerManager = World->GetTimerManager();

	// 패링 가능 시간 타이머
	TimerManager.ClearTimer(ParryTimerHandle);
	TimerManager.SetTimer(
		ParryTimerHandle,
		this,
		&UCombatComponent::CloseParryWindow,
		ParryWindow,
		false
	);

	// 다음 패링 시도까지의 쿨타임
	TimerManager.ClearTimer(ParryCooldownTimerHandle);
	TimerManager.SetTimer(
		ParryCooldownTimerHandle,
		this,
		&UCombatComponent::EndParryCooldown,
		ParryCooldown,
		false
	);
}

void UCombatComponent::CloseParryWindow()
{
	bParryWindowOpen = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryTimerHandle);
	}
}

void UCombatComponent::EndParryCooldown()
{
	bParryOnCooldown = false;
}


void UCombatComponent::SetActionState(EPlayerActionState State)
{
	if (ActionState == State) return;

	const EPlayerActionState PreviousState = ActionState;

	if (bSwordApproaching && State != EPlayerActionState::AttackingWithSword)
	{
		FinishSwordApproach(false);
	}

	const bool bEnteringSwordRecovery =
		PreviousState == EPlayerActionState::AttackingWithSword &&
		State == EPlayerActionState::None &&
		bSwordRecovery;

	if (bSwordRecovery && !bEnteringSwordRecovery)
	{
		CancelSwordRecovery();
	}

	if (PreviousState == EPlayerActionState::AttackingWithSword && !bEnteringSwordRecovery)
	{
		ResetSwordCombo();
		EndSwordDamageWindow();
	}

	ActionState = State;

	OnActionStateChangedEvent.Broadcast(PreviousState, ActionState);

	OnActionStateChanged(PreviousState, ActionState);
}

void UCombatComponent::ConsumeBlockStamina()
{
	if (ActionState != EPlayerActionState::Blocking || !IsValid(StatComponent))
	{
		StopBlock();
		return;
	}

	if (!TrySpendStamina(BlockHoldStaminaCost))
	{
		StopBlock();
		return;
	}

	// 다음 공격을 막을 스태미나가 없으면 가드 자동 해제
	if (!StatComponent->HasEnoughStamina(BlockStaminaCost))
	{
		StopBlock();
	}
}

void UCombatComponent::StartHitReaction()
{
	if (!IsOwnerAlive() || !IsValid(OwnerPlayer))
	{
		return;
	}

	UAnimInstance* AnimInstance =
		OwnerPlayer->GetMesh()
		? OwnerPlayer->GetMesh()->GetAnimInstance()
		: nullptr;

	if (!IsValid(AnimInstance))
	{
		return;
	}

	// 피해를 받기 전 행동을 저장합니다.
	const EPlayerActionState PreviousState = ActionState;

	// 가드 중이었다면 가드 타이머와 패링 창을 정리합니다.
	if (PreviousState == EPlayerActionState::Blocking)
	{
		StopBlock();
	}

	// 공격 중이었다면 검의 피해 판정을 즉시 종료합니다.
	if (PreviousState == EPlayerActionState::AttackingWithSword)
	{
		EndSwordDamageWindow();
	}

	// 구르기 중이었다면 구르기 이동을 즉시 제거합니다.
	if (PreviousState == EPlayerActionState::Rolling)
	{
		if (UCharacterMovementComponent* Movement =
			OwnerPlayer->GetCharacterMovement())
		{
			Movement->RemoveRootMotionSource(
				RollRootMotionSourceName
			);
		}
	}

	// 기존 몽타주 종료 콜백이 이동을 다시 켜지 못하도록
	// 먼저 상태를 HitReact로 변경합니다.
	OwnerPlayer->SetCanMove(false);
	SetActionState(EPlayerActionState::HitReact);

	// 현재 재생 중인 행동 몽타주를 정지합니다.
	if (PreviousState == EPlayerActionState::AttackingWithSword &&
		IsValid(SwordAttackMontage) &&
		AnimInstance->Montage_IsPlaying(SwordAttackMontage))
	{
		AnimInstance->Montage_Stop(
			0.1f,
			SwordAttackMontage
		);
	}

	if (PreviousState == EPlayerActionState::Rolling &&
		IsValid(RollMontage) &&
		AnimInstance->Montage_IsPlaying(RollMontage))
	{
		AnimInstance->Montage_Stop(
			0.1f,
			RollMontage
		);
	}

	// 피격 몽타주가 없으면 상태가 잠기지 않도록 복구합니다.
	if (!IsValid(HitReactMontage))
	{
		OwnerPlayer->SetCanMove(true);
		FinishAction(EPlayerActionState::HitReact);
		return;
	}

	const float PlayedLength =
		OwnerPlayer->PlayAnimMontage(HitReactMontage);

	if (PlayedLength <= 0.0f)
	{
		OwnerPlayer->SetCanMove(true);
		FinishAction(EPlayerActionState::HitReact);
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&UCombatComponent::OnHitReactMontageEnded
	);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		HitReactMontage
	);
}

void UCombatComponent::OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != HitReactMontage)
	{
		return;
	}

	// 다른 피격이나 사망으로 몽타주가 교체됐다면
	// 여기서 이동을 다시 허용하지 않습니다.
	if (bInterrupted)
	{
		return;
	}

	if (ActionState != EPlayerActionState::HitReact)
	{
		return;
	}

	if (IsValid(OwnerPlayer))
	{
		OwnerPlayer->SetCanMove(true);
	}

	FinishAction(EPlayerActionState::HitReact);
}
