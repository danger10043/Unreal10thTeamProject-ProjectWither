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
}

void UCombatComponent::SwordAttack()
{
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
		OwnerPlayer->SetCanMove(true);
	}
	FinishAction(EPlayerActionState::Rolling);
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
	return false;
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

	if (!OwnerPlayer->CanMove())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::CanRoll 실패: PlayerCharacter의 bCanMove가 false입니다."));
		return false;
	}

	if (ActionState != EPlayerActionState::None)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::CanRoll 실패: 현재 ActionState는 %d입니다."),
			static_cast<int32>(ActionState)
		);
		return false;
	}

	if (!StatComponent->HasEnoughStamina(RollStaminaCost))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("UCombatComponent::CanRoll 실패: 스태미나가 부족합니다. 현재 %.1f / 필요 %.1f"),
			StatComponent->GetCurrentStamina(),
			RollStaminaCost
		);
		return false;
	}

	const UCharacterMovementComponent* Movement =
		OwnerPlayer->GetCharacterMovement();

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
	return IsValid(OwnerPlayer)
		&& IsValid(StatComponent)
		&& !StatComponent->IsHealthZero()
		&& ActionState != EPlayerActionState::Dead;
}

bool UCombatComponent::TrySpendStamina(float Cost)
{
	if (Cost <= 0.0f) return true;
	if (!IsValid(StatComponent)) return false;

	return StatComponent->UseStamina(Cost) >= Cost;
}

void UCombatComponent::StartAttack(ECombatWeaponType RequiredWeapon, EPlayerActionState AttackState, float StaminaCost)
{
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
