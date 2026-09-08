#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/StatUpgradeTypes.h"
#include "StatUpgradeComponent.generated.h"

class UInventoryComponent;
class UStatComponent;
class UStatUpgradeSettingsDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUpgradeLevelChangedDelegate, EUpgradeableStatType, StatType, int32, NewLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTWITHER_API UStatUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatUpgradeComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Stat Upgrade|Event")
	FOnStatUpgradeLevelChangedDelegate OnStatUpgradeLevelChanged;

	UFUNCTION(BlueprintCallable, Category = "Stat Upgrade")
	EStatUpgradeResult TryUpgradeStat(EUpgradeableStatType StatType);

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	bool CanUpgradeStat(EUpgradeableStatType StatType, EStatUpgradeResult& OutResult) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	int32 GetUpgradeLevel(EUpgradeableStatType StatType) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	int32 GetMaxUpgradeLevel(EUpgradeableStatType StatType) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	bool IsMaxUpgradeLevel(EUpgradeableStatType StatType) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	bool GetNextUpgradeCost(EUpgradeableStatType StatType, FStatUpgradeCost& OutCost) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	float GetIncreaseAmountPerLevel(EUpgradeableStatType StatType) const;

private:
	const FStatUpgradeRuleSet& GetActiveRules() const;
	const FStatUpgradeRule* FindRule(EUpgradeableStatType StatType) const;
	EStatUpgradeResult ValidateUpgrade(EUpgradeableStatType StatType, const FStatUpgradeRule*& OutRule, FStatUpgradeCost& OutCost) const;
	EStatUpgradeResult SpendUpgradeCost(const FStatUpgradeCost& Cost);
	void ApplyUpgradeEffect(EUpgradeableStatType StatType, float IncreaseAmount);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStatUpgradeSettingsDataAsset> UpgradeSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	FStatUpgradeRuleSet InlineRules;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	TMap<EUpgradeableStatType, int32> UpgradeLevels;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;
};
