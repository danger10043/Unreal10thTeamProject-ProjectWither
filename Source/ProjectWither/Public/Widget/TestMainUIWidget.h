#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestMainUIWidget.generated.h"

class USegmentedStatBarWidget;
class UStatComponent;
class UCombatComponent;
class UAmmoCountWidget;
class UWeaponComponent;
class UWeaponTypeWidget;
class UPotionCountWidget;

UCLASS()
class PROJECTWITHER_API UTestMainUIWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindPlayerComponents();
	void UnbindPlayerComponents();
	void InitializeWidgetValues();

	UFUNCTION()
	void HandleWeaponChanged();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina, float ChangedAmount);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USegmentedStatBarWidget> HealthBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USegmentedStatBarWidget> StaminaBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAmmoCountWidget> AmmoCountWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponTypeWidget> WeaponTypeWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPotionCountWidget> PotionCountWidget;

private:
	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponComponent> WeaponComponent;

};
