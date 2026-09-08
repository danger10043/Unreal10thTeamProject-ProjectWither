#pragma once

#include "CoreMinimal.h"
#include "StatUpgradeTypes.generated.h"

UENUM(BlueprintType)
enum class EUpgradeableStatType : uint8
{
	Health			UMETA(DisplayName = "Health"),
	Stamina			UMETA(DisplayName = "Stamina"),
	AttackPower		UMETA(DisplayName = "Attack Power"),
	DefensePower	UMETA(DisplayName = "Defense Power")
};

UENUM(BlueprintType)
enum class EStatUpgradeResult : uint8
{
	Success					UMETA(DisplayName = "Success"),
	MissingStatComponent	UMETA(DisplayName = "Missing Stat Component"),
	MissingInventoryComponent UMETA(DisplayName = "Missing Inventory Component"),
	InvalidUpgradeSettings	UMETA(DisplayName = "Invalid Upgrade Settings"),
	MaxLevelReached			UMETA(DisplayName = "Max Level Reached"),
	NotEnoughGold			UMETA(DisplayName = "Not Enough Gold")
};

USTRUCT(BlueprintType)
struct FStatUpgradeCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade", meta = (ClampMin = "0"))
	int32 Gold = 0;
};

USTRUCT(BlueprintType)
struct FStatUpgradeRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade", meta = (ClampMin = "0"))
	int32 MaxLevel = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade", meta = (ClampMin = "0.0"))
	float IncreaseAmountPerLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade", meta = (ClampMin = "0"))
	int32 BaseGoldCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade", meta = (ClampMin = "0"))
	int32 GoldCostIncreasePerLevel = 50;

	bool IsValidRule() const
	{
		return MaxLevel > 0 && IncreaseAmountPerLevel > 0.0f;
	}

	FStatUpgradeCost GetCostForCurrentLevel(int32 CurrentLevel) const
	{
		const int32 NormalizedCurrentLevel = FMath::Max(0, CurrentLevel);

		FStatUpgradeCost Cost;
		Cost.Gold = FMath::Max(0, BaseGoldCost + GoldCostIncreasePerLevel * NormalizedCurrentLevel);
		return Cost;
	}
};

USTRUCT(BlueprintType)
struct FStatUpgradeRuleSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade")
	FStatUpgradeRule Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade")
	FStatUpgradeRule Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade")
	FStatUpgradeRule AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat Upgrade")
	FStatUpgradeRule DefensePower;

	const FStatUpgradeRule* FindRule(EUpgradeableStatType StatType) const
	{
		switch (StatType)
		{
		case EUpgradeableStatType::Health:
			return &Health;
		case EUpgradeableStatType::Stamina:
			return &Stamina;
		case EUpgradeableStatType::AttackPower:
			return &AttackPower;
		case EUpgradeableStatType::DefensePower:
			return &DefensePower;
		default:
			return nullptr;
		}
	}

	void ApplyDefaultValues()
	{
		Health.MaxLevel = 10;
		Health.IncreaseAmountPerLevel = 15.0f;
		Health.BaseGoldCost = 100;
		Health.GoldCostIncreasePerLevel = 50;

		Stamina.MaxLevel = 10;
		Stamina.IncreaseAmountPerLevel = 10.0f;
		Stamina.BaseGoldCost = 100;
		Stamina.GoldCostIncreasePerLevel = 50;

		AttackPower.MaxLevel = 10;
		AttackPower.IncreaseAmountPerLevel = 2.0f;
		AttackPower.BaseGoldCost = 150;
		AttackPower.GoldCostIncreasePerLevel = 75;

		DefensePower.MaxLevel = 10;
		DefensePower.IncreaseAmountPerLevel = 2.0f;
		DefensePower.BaseGoldCost = 150;
		DefensePower.GoldCostIncreasePerLevel = 75;
	}
};
