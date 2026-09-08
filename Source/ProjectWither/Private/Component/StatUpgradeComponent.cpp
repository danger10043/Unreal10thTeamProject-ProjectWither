#include "Component/StatUpgradeComponent.h"

#include "Component/InventoryComponent.h"
#include "Component/StatComponent.h"
#include "DataAsset/StatUpgradeSettingsDataAsset.h"

UStatUpgradeComponent::UStatUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InlineRules.ApplyDefaultValues();
}

void UStatUpgradeComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
	{
		return;
	}

	StatComponent = OwnerActor->FindComponentByClass<UStatComponent>();
	InventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();
}

EStatUpgradeResult UStatUpgradeComponent::TryUpgradeStat(EUpgradeableStatType StatType)
{
	const FStatUpgradeRule* Rule = nullptr;
	FStatUpgradeCost Cost;

	const EStatUpgradeResult ValidationResult = ValidateUpgrade(StatType, Rule, Cost);

	if (ValidationResult != EStatUpgradeResult::Success)
	{
		return ValidationResult;
	}

	const EStatUpgradeResult SpendResult = SpendUpgradeCost(Cost);

	if (SpendResult != EStatUpgradeResult::Success)
	{
		return SpendResult;
	}

	int32& CurrentLevel = UpgradeLevels.FindOrAdd(StatType);
	CurrentLevel = FMath::Max(0, CurrentLevel) + 1;

	ApplyUpgradeEffect(StatType, Rule->IncreaseAmountPerLevel);

	OnStatUpgradeLevelChanged.Broadcast(StatType, CurrentLevel);

	return EStatUpgradeResult::Success;
}

bool UStatUpgradeComponent::CanUpgradeStat(EUpgradeableStatType StatType, EStatUpgradeResult& OutResult) const
{
	const FStatUpgradeRule* Rule = nullptr;
	FStatUpgradeCost Cost;

	OutResult = ValidateUpgrade(StatType, Rule, Cost);
	return OutResult == EStatUpgradeResult::Success;
}

int32 UStatUpgradeComponent::GetUpgradeLevel(EUpgradeableStatType StatType) const
{
	const int32* Level = UpgradeLevels.Find(StatType);
	return Level ? FMath::Max(0, *Level) : 0;
}

int32 UStatUpgradeComponent::GetMaxUpgradeLevel(EUpgradeableStatType StatType) const
{
	const FStatUpgradeRule* Rule = FindRule(StatType);
	return Rule ? FMath::Max(0, Rule->MaxLevel) : 0;
}

bool UStatUpgradeComponent::IsMaxUpgradeLevel(EUpgradeableStatType StatType) const
{
	const FStatUpgradeRule* Rule = FindRule(StatType);
	return Rule && GetUpgradeLevel(StatType) >= Rule->MaxLevel;
}

bool UStatUpgradeComponent::GetNextUpgradeCost(EUpgradeableStatType StatType, FStatUpgradeCost& OutCost) const
{
	const FStatUpgradeRule* Rule = FindRule(StatType);

	if (!Rule || !Rule->IsValidRule() || IsMaxUpgradeLevel(StatType))
	{
		OutCost = FStatUpgradeCost();
		return false;
	}

	OutCost = Rule->GetCostForCurrentLevel(GetUpgradeLevel(StatType));
	return true;
}

float UStatUpgradeComponent::GetIncreaseAmountPerLevel(EUpgradeableStatType StatType) const
{
	const FStatUpgradeRule* Rule = FindRule(StatType);
	return Rule ? FMath::Max(0.0f, Rule->IncreaseAmountPerLevel) : 0.0f;
}

const FStatUpgradeRuleSet& UStatUpgradeComponent::GetActiveRules() const
{
	return IsValid(UpgradeSettings) ? UpgradeSettings->GetRules() : InlineRules;
}

const FStatUpgradeRule* UStatUpgradeComponent::FindRule(EUpgradeableStatType StatType) const
{
	return GetActiveRules().FindRule(StatType);
}

EStatUpgradeResult UStatUpgradeComponent::ValidateUpgrade(
	EUpgradeableStatType StatType,
	const FStatUpgradeRule*& OutRule,
	FStatUpgradeCost& OutCost) const
{
	OutRule = nullptr;
	OutCost = FStatUpgradeCost();

	if (!IsValid(StatComponent))
	{
		return EStatUpgradeResult::MissingStatComponent;
	}

	if (!IsValid(InventoryComponent))
	{
		return EStatUpgradeResult::MissingInventoryComponent;
	}

	const FStatUpgradeRule* Rule = FindRule(StatType);

	if (!Rule || !Rule->IsValidRule())
	{
		return EStatUpgradeResult::InvalidUpgradeSettings;
	}

	const int32 CurrentLevel = GetUpgradeLevel(StatType);

	if (CurrentLevel >= Rule->MaxLevel)
	{
		return EStatUpgradeResult::MaxLevelReached;
	}

	const FStatUpgradeCost Cost = Rule->GetCostForCurrentLevel(CurrentLevel);

	if (!InventoryComponent->HasEnoughGold(Cost.Gold))
	{
		return EStatUpgradeResult::NotEnoughGold;
	}

	OutRule = Rule;
	OutCost = Cost;
	return EStatUpgradeResult::Success;
}

EStatUpgradeResult UStatUpgradeComponent::SpendUpgradeCost(const FStatUpgradeCost& Cost)
{
	if (!IsValid(InventoryComponent))
	{
		return EStatUpgradeResult::MissingInventoryComponent;
	}

	if (Cost.Gold > 0 && !InventoryComponent->SpendGold(Cost.Gold))
	{
		return EStatUpgradeResult::NotEnoughGold;
	}

	return EStatUpgradeResult::Success;
}

void UStatUpgradeComponent::ApplyUpgradeEffect(EUpgradeableStatType StatType, float IncreaseAmount)
{
	if (!IsValid(StatComponent) || IncreaseAmount <= 0.0f)
	{
		return;
	}

	switch (StatType)
	{
	case EUpgradeableStatType::Health:
		StatComponent->AddMaxHealth(IncreaseAmount);
		break;
	case EUpgradeableStatType::Stamina:
		StatComponent->AddMaxStamina(IncreaseAmount);
		break;
	case EUpgradeableStatType::AttackPower:
		StatComponent->AddAttackPower(IncreaseAmount, IncreaseAmount);
		break;
	case EUpgradeableStatType::DefensePower:
		StatComponent->AddDefensePower(IncreaseAmount);
		break;
	default:
		break;
	}
}
