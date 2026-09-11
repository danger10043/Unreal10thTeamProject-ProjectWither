#include "Widget/AmmoCountWidget.h"

#include "Components/TextBlock.h"

void UAmmoCountWidget::SetAmmo(int32 CurrentAmmo, int32 MaxAmmo)
{
	const int32 SafeMaxAmmo = FMath::Max(0, MaxAmmo);
	const int32 SafeCurrentAmmo = FMath::Clamp(
		CurrentAmmo,
		0,
		SafeMaxAmmo
	);

	if (IsValid(CurrentAmmoText))
	{
		CurrentAmmoText->SetText(FText::AsNumber(SafeCurrentAmmo));
	}

	if (IsValid(MaxAmmoText))
	{
		MaxAmmoText->SetText(FText::AsNumber(SafeMaxAmmo));
	}
}