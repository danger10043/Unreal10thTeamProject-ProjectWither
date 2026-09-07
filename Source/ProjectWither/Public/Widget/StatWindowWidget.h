#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatWindowWidget.generated.h"

class UEquipmentComponent;
class UStatComponent;
class UTextBlock;
class UWeaponComponent;

UCLASS()
class PROJECTWITHER_API UStatWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Stat")
	void RefreshStats();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindPlayerComponents();
	void UnbindPlayerComponents();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthStatText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AttackPowerStatText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageStatText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DefensePowerStatText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AttackPowerValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageValueText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DefensePowerValueText;

private:
	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponComponent> WeaponComponent;
};
