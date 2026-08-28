#include "Component/CombatComponent.h"

#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Component/WeaponComponent.h"
#include "Engine/World.h"

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
}

bool UCombatComponent::CanAttack() const
{
	return false;
}

bool UCombatComponent::CanRoll() const
{
	return false;
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
	return false;
}

bool UCombatComponent::TrySpendStamina(float Cost)
{
	return false;
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
}
