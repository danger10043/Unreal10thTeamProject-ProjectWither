#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/StatUpgradeTypes.h"
#include "StatUpgradeWindowWidget.generated.h"

class UButton;
class UInventoryComponent;
class UStatComponent;
class UStatUpgradeComponent;
class UTextBlock;

UCLASS()
class PROJECTWITHER_API UStatUpgradeWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Stat Upgrade")
	void RefreshUpgradeInfo();

	UFUNCTION(BlueprintCallable, Category = "UI|Stat Upgrade")
	EStatUpgradeResult TryUpgradeHealth();

	UFUNCTION(BlueprintCallable, Category = "UI|Stat Upgrade")
	EStatUpgradeResult TryUpgradeStamina();

	UFUNCTION(BlueprintCallable, Category = "UI|Stat Upgrade")
	EStatUpgradeResult TryUpgradeAttackPower();

	UFUNCTION(BlueprintCallable, Category = "UI|Stat Upgrade")
	EStatUpgradeResult TryUpgradeDefensePower();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindPlayerComponents();
	void UnbindPlayerComponents();
	void BindButtons();
	void UnbindButtons();

	EStatUpgradeResult TryUpgrade(EUpgradeableStatType StatType);
	void RefreshUpgradeRow(EUpgradeableStatType StatType, UTextBlock* LevelText, UTextBlock* CostText) const;
	void RefreshOwnedCurrencyText() const;
	void SetResultText(EStatUpgradeResult Result);

	UFUNCTION()
	void HandleHealthUpgradeClicked();

	UFUNCTION()
	void HandleStaminaUpgradeClicked();

	UFUNCTION()
	void HandleAttackPowerUpgradeClicked();

	UFUNCTION()
	void HandleDefensePowerUpgradeClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleUpgradeLevelChanged(EUpgradeableStatType StatType, int32 NewLevel);

	UFUNCTION()
	void HandleGoldChanged(int32 CurrentGold, int32 ChangedAmount);

	UFUNCTION()
	void HandleStatsChanged();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> HealthUpgradeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> StaminaUpgradeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AttackPowerUpgradeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> DefensePowerUpgradeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthLevelText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthCostText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StaminaLevelText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StaminaCostText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AttackPowerLevelText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AttackPowerCostText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DefensePowerLevelText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DefensePowerCostText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultText;

private:
	UPROPERTY(Transient)
	TObjectPtr<UStatUpgradeComponent> StatUpgradeComponent;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;
};
