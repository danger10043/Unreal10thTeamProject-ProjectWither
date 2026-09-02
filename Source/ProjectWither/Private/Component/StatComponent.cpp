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

void UStatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaRecoveryDelayTimerHandle);
		World->GetTimerManager().ClearTimer(StaminaRecoveryTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UStatComponent::ResetStat()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(StaminaRecoveryDelayTimerHandle);
		TimerManager.ClearTimer(StaminaRecoveryTimerHandle);
	}

	const float PreviousHealth = CurrentHealth;
	const float PreviousStamina = CurrentStamina;

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;

	OnHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth,
		CurrentHealth - PreviousHealth);

	OnStaminaChanged.Broadcast(
		CurrentStamina,
		MaxStamina,
		CurrentStamina - PreviousStamina);
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

	RestartStaminaRecoveryDelay();

	return Amount;
}

void UStatComponent::RestartStaminaRecoveryDelay()
{
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	FTimerManager& TimerManager = World->GetTimerManager();

	TimerManager.ClearTimer(StaminaRecoveryDelayTimerHandle);
	TimerManager.ClearTimer(StaminaRecoveryTimerHandle);

	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	if (StaminaRecoveryDelay <= 0.0f)
	{
		StartStaminaRecovery();
		return;
	}

	TimerManager.SetTimer(
		StaminaRecoveryDelayTimerHandle,
		this,
		&UStatComponent::StartStaminaRecovery,
		StaminaRecoveryDelay,
		false
	);
}

void UStatComponent::StartStaminaRecovery()
{
	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	if (StaminaRecoveryAmountPerTick <= 0.0f || StaminaRecoveryInterval <= 0.0f)
	{
		return;
	}

	RecoverStaminaTick();

	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		StaminaRecoveryTimerHandle,
		this,
		&UStatComponent::RecoverStaminaTick,
		StaminaRecoveryInterval,
		true
	);
}

void UStatComponent::RecoverStaminaTick()
{
	RecoverStamina(StaminaRecoveryAmountPerTick);

	if (CurrentStamina >= MaxStamina)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StaminaRecoveryTimerHandle);
		}
	}
}


