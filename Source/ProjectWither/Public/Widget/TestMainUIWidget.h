#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonHeader/PlayerActionStateEnums.h"
#include "TestMainUIWidget.generated.h"

class UProgressBarBaseWidget;
class UCurrentStateWidget;
class UStatComponent;
class UCombatComponent;

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
	void InitializeWidgetColors();

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina, float ChangedAmount);

	UFUNCTION()
	void HandleActionStateChanged(EPlayerActionState PreviousState, EPlayerActionState NewState);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBarBaseWidget> HealthBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBarBaseWidget> StaminaBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCurrentStateWidget> CurrentStateWidget;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

};
