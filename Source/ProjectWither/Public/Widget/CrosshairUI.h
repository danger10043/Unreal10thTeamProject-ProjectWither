#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairUI.generated.h"

class UImage;
class UWidgetAnimation;
class UWeaponComponent;
class UPlayerCameraComponent;

UCLASS()
class PROJECTWITHER_API UCrosshairUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrossHairImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ZoomStartAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ZoomEndAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> GunFireAnim;

private:
	UFUNCTION()
	void HandleWeaponChanged();

	UFUNCTION()
	void HandleZoomChanged(bool bIsZooming);

	UFUNCTION()
	void HandleGunFired();

	void ResetCrosshair();

	UPROPERTY(Transient)
	TObjectPtr<UWeaponComponent> WeaponComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPlayerCameraComponent> CameraComponent;

	FWidgetTransform DefaultImageTransform;
	float DefaultImageOpacity = 1.0f;

	bool bGunEquipped = false;
	bool bZooming = false;
};