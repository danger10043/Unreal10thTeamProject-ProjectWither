#include "Widget/CrosshairUI.h"

#include "Animation/WidgetAnimation.h"
#include "Component/PlayerCameraComponent.h"
#include "Component/WeaponComponent.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"

void UCrosshairUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(CrossHairImage))
	{
		DefaultImageTransform = CrossHairImage->GetRenderTransform();
		DefaultImageOpacity = CrossHairImage->GetRenderOpacity();
	}

	bGunEquipped = false;
	bZooming = false;

	APawn* OwningPawn = GetOwningPlayerPawn();

	if (IsValid(OwningPawn))
	{
		WeaponComponent =
			OwningPawn->FindComponentByClass<UWeaponComponent>();

		CameraComponent =
			OwningPawn->FindComponentByClass<UPlayerCameraComponent>();
	}

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.AddUniqueDynamic(
			this,
			&UCrosshairUI::HandleWeaponChanged
		);

		WeaponComponent->OnGunFired.AddUniqueDynamic(
			this,
			&UCrosshairUI::HandleGunFired
		);
	}

	if (IsValid(CameraComponent))
	{
		CameraComponent->OnZoomChanged.AddUniqueDynamic(
			this,
			&UCrosshairUI::HandleZoomChanged
		);
	}

	HandleWeaponChanged();
}

void UCrosshairUI::NativeDestruct()
{
	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.RemoveDynamic(
			this,
			&UCrosshairUI::HandleWeaponChanged
		);

		WeaponComponent->OnGunFired.RemoveDynamic(
			this,
			&UCrosshairUI::HandleGunFired
		);
	}

	if (IsValid(CameraComponent))
	{
		CameraComponent->OnZoomChanged.RemoveDynamic(
			this,
			&UCrosshairUI::HandleZoomChanged
		);
	}

	ResetCrosshair();

	WeaponComponent = nullptr;
	CameraComponent = nullptr;
	bGunEquipped = false;
	bZooming = false;

	Super::NativeDestruct();
}

void UCrosshairUI::ResetCrosshair()
{
	StopAllAnimations();

	if (IsValid(CrossHairImage))
	{
		CrossHairImage->SetRenderTransform(DefaultImageTransform);
		CrossHairImage->SetRenderOpacity(DefaultImageOpacity);
	}
}

void UCrosshairUI::HandleWeaponChanged()
{
	const bool bNowGunEquipped =
		IsValid(WeaponComponent) && WeaponComponent->IsGunEquipped();

	if (!bNowGunEquipped)
	{
		ResetCrosshair();

		bGunEquipped = false;
		bZooming = false;

		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (!bGunEquipped)
	{
		ResetCrosshair();
		bZooming = false;
	}

	bGunEquipped = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	const bool bNowZooming =
		IsValid(CameraComponent) && CameraComponent->IsZooming();

	if (bZooming == bNowZooming) return;

	bZooming = bNowZooming;

	if (IsValid(ZoomStartAnim))
	{
		StopAnimation(ZoomStartAnim);
	}

	if (IsValid(ZoomEndAnim))
	{
		StopAnimation(ZoomEndAnim);
	}

	UWidgetAnimation* ZoomAnimation =
		bZooming ? ZoomStartAnim.Get() : ZoomEndAnim.Get();

	if (IsValid(ZoomAnimation))
	{
		PlayAnimation(
			ZoomAnimation,
			0.0f,
			1,
			EUMGSequencePlayMode::Forward,
			1.0f,
			false
		);
	}
}

void UCrosshairUI::HandleZoomChanged(bool bIsZooming)
{
	// 무기 상태와 실제 카메라 상태를 함께 확인한다.
	HandleWeaponChanged();
}

void UCrosshairUI::HandleGunFired()
{
	HandleWeaponChanged();

	if (!bGunEquipped || !IsValid(GunFireAnim)) return;

	StopAnimation(GunFireAnim);

	PlayAnimation(
		GunFireAnim,
		0.0f,
		1,
		EUMGSequencePlayMode::Forward,
		1.0f,
		true
	);
}