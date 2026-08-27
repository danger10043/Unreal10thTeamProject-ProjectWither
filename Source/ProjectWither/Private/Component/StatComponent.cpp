#include "Component/StatComponent.h"

UStatComponent::UStatComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxHealth = FMath::Max(0.0f, MaxHealth);
	MaxStamina = FMath::Max(0.0f, MaxStamina);

	if (MinAttackPower > MaxAttackPower)
	{
		Swap(MinAttackPower, MaxAttackPower);
	}

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
}

float UStatComponent::RecoverHealth(float Amount)
{
	if (Amount <= 0.0f || IsHealthZero()) return 0.0f;

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);

	const float RecoveredAmount = CurrentHealth - PreviousHealth;

	if (RecoveredAmount > 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, RecoveredAmount);
	}

	return RecoveredAmount;
}

float UStatComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f || IsHealthZero()) return 0.0f;

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	const float AppliedDamage = PreviousHealth - CurrentHealth;

	if (AppliedDamage > 0.0f)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, -AppliedDamage);
	}

	if (PreviousHealth > 0.0f && IsHealthZero())
	{
		OnHealthZero.Broadcast();
	}
	
	return AppliedDamage;
}

float UStatComponent::RecoverStamina(float Amount)
{
	if (Amount <= 0.0f) return 0.0f;

	const float PreviousStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

	const float RecoveredAmount = CurrentStamina - PreviousStamina;

	if (RecoveredAmount > 0.0f)
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, RecoveredAmount);
	}

	return RecoveredAmount;
}

float UStatComponent::UseStamina(float Amount)
{
	if (Amount <= 0.0f || !HasEnoughStamina(Amount)) return 0.0f;

	CurrentStamina -= Amount;

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina, -Amount);

	return Amount;
}


