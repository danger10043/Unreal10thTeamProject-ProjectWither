#include "Component/CombatComponent.h"

#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Component/WeaponComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

namespace
{
	const FName RollRootMotionSourceName(TEXT("CombatRoll"));
}

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = Cast<APlayerCharacter>(GetOwner());

	if (!ensureMsgf(
		IsValid(OwnerPlayer),
		TEXT("CombatComponent의 PlayerCharacter가 유효하지 않습니다.")
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryTimerHandle);
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
		UE_LOG( LogTemp, Warning, TEXT("장착된 무기가 없어 공격할 수 없습니다."));
		break;
	}
}

void UCombatComponent::SwordAttack()
{
	StartAttack( ECombatWeaponType::Sword, EPlayerActionState::AttackingWithSword, SwordAttackStaminaCost);
}

void UCombatComponent::GunAttack()
{
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

void UCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != SwordAttackMontage) { return; }

	if (ActionState != EPlayerActionState::AttackingWithSword) { return; }

	if (IsValid(OwnerPlayer))
	{
		OwnerPlayer->SetCanMove(true);
	}

	FinishAction(EPlayerActionState::AttackingWithSword);
}

void UCombatComponent::StartBlock()
{
}

void UCombatComponent::StopBlock()
{
}

void UCombatComponent::ReceiveHit()
{
}

void UCombatComponent::Die()
{
}

void UCombatComponent::FinishAction(EPlayerActionState ExpectedState)
{
	if (ActionState != ExpectedState) return;

	SetActionState(EPlayerActionState::None);
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
	return false;
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
		UE_LOG(LogTemp, Warning, TEXT("OwnerPlayer 가 유효하지 않습니다."));
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

	if (!TrySpendStamina(StaminaCost))
	{
		UE_LOG( LogTemp, Warning, TEXT("공격에 필요한 스태미나가 부족합니다."));
		return;
	}

	// SetCanMove(false) 내부에서 달리기도 종료됩니다.
	OwnerPlayer->SetCanMove(false);
	SetActionState(AttackState);

	const float PlayedLength = OwnerPlayer->PlayAnimMontage(SwordAttackMontage);

	if (PlayedLength <= 0.0f)
	{
		StatComponent->RecoverStamina(StaminaCost);
		OwnerPlayer->SetCanMove(true);
		FinishAction(AttackState);
		return;
	}

	FOnMontageEnded EndDelegate;

	EndDelegate.BindUObject( this, &UCombatComponent::OnAttackMontageEnded);

	AnimInstance->Montage_SetEndDelegate( EndDelegate, SwordAttackMontage);

	OnAttackStarted(RequiredWeapon);
}

void UCombatComponent::OpenParryWindow()
{
}

void UCombatComponent::CloseParryWindow()
{
}


void UCombatComponent::SetActionState(EPlayerActionState State)
{
	if (ActionState == State) return;

	const EPlayerActionState PreviousState = ActionState;
	ActionState = State;

	OnActionStateChanged(PreviousState, ActionState);
}
