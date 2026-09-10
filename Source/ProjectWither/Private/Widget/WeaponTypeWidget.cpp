#include "Widget/WeaponTypeWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UWeaponTypeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshWeaponImage();
}

void UWeaponTypeWidget::SetWeaponType(EWeaponType WeaponType)
{
	CurrentWeaponType = WeaponType;

	RefreshWeaponImage();
}

void UWeaponTypeWidget::RefreshWeaponImage()
{
	if (!IsValid(WeaponTypeImage)) return;

	UTexture2D* SelectedTexture = nullptr;

	switch (CurrentWeaponType)
	{
	case EWeaponType::Sword:
		SelectedTexture = WeaponSword.Get();
		break;

	case EWeaponType::Gun:
		SelectedTexture = WeaponGun.Get();
		break;

	case EWeaponType::None:
	default:
		SelectedTexture = WeaponNone.Get();
		break;
	}

	WeaponTypeImage->SetBrushFromTexture(SelectedTexture, false);
}