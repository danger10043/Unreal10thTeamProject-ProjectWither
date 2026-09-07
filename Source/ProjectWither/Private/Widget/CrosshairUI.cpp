#include "Widget/CrosshairUI.h"

#include "Component/WeaponComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"
#include "Interface/WeaponComponentUserInterface.h"

void UCrosshairUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::HitTestInvisible);

	CurrentGap = FMath::Max(0.0f, MinGap);
	SetCrosshairVisible(false);
	UpdateCrosshair(0.0f);
}

void UCrosshairUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	UpdateCrosshair(DeltaTime);
}

void UCrosshairUI::UpdateCrosshair(float DeltaTime)
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	UWeaponComponent* WeaponComponent = nullptr;

	if (
		IsValid(OwningPawn) &&
		OwningPawn->GetClass()->ImplementsInterface(
			UWeaponComponentUserInterface::StaticClass())
		)
	{
		WeaponComponent =
			IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwningPawn);
	}

	const bool bShouldShow =
		IsValid(WeaponComponent) && WeaponComponent->IsGunEquipped();

	const float SafeMinGap = FMath::Max(0.0f, MinGap);
	const float SafeMaxGap = FMath::Max(SafeMinGap, MaxGap);

	if (!bShouldShow)
	{
		CurrentGap = SafeMinGap;
		SetCrosshairVisible(false);
		return;
	}

	const float Speed = OwningPawn->GetVelocity().Size2D();

	const float SpeedAlpha = FMath::Clamp(
		Speed / FMath::Max(1.0f, SpeedForMaxSpread),
		0.0f,
		1.0f
	);
	
	const float TargetGap = FMath::Lerp(SafeMinGap, SafeMaxGap, SpeedAlpha);

	if (!bCrosshairVisible)
	{
		CurrentGap = TargetGap;
	}
	else
	{
		CurrentGap = FMath::FInterpTo(
			CurrentGap,
			TargetGap,
			DeltaTime,
			FMath::Max(0.1f, SpreadInterpSpeed)
		);
	}

	ApplyCrosshairGap();
	SetCrosshairVisible(true);
}

void UCrosshairUI::SetCrosshairVisible(bool bVisible)
{
	bCrosshairVisible = bVisible;

	const ESlateVisibility NewVisibility =
		bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;

	UImage* Images[] =
	{
		CenterDot.Get(),
		TopBar.Get(),
		BottomBar.Get(),
		LeftBar.Get(),
		RightBar.Get()
	};

	for (UImage* Image : Images)
	{
		if (IsValid(Image))
		{
			Image->SetVisibility(NewVisibility);
		}
	}
}

void UCrosshairUI::ApplyCrosshairGap()
{
	if (!IsValid(CenterDot) ||
		!IsValid(TopBar) ||
		!IsValid(BottomBar) ||
		!IsValid(LeftBar) ||
		!IsValid(RightBar))
	{
		return;
	}

	const UCanvasPanelSlot* DotSlot =
		Cast<UCanvasPanelSlot>(CenterDot->Slot);
	const UCanvasPanelSlot* TopSlot =
		Cast<UCanvasPanelSlot>(TopBar->Slot);
	const UCanvasPanelSlot* BottomSlot =
		Cast<UCanvasPanelSlot>(BottomBar->Slot);
	const UCanvasPanelSlot* LeftSlot =
		Cast<UCanvasPanelSlot>(LeftBar->Slot);
	const UCanvasPanelSlot* RightSlot =
		Cast<UCanvasPanelSlot>(RightBar->Slot);

	if (!DotSlot || !TopSlot || !BottomSlot || !LeftSlot || !RightSlot)
	{
		return;
	}

	const FVector2D DotHalfSize = DotSlot->GetSize() * 0.5f;

	const float TopDistance =
		DotHalfSize.Y + CurrentGap + TopSlot->GetSize().Y * 0.5;
	const float BottomDistance =
		DotHalfSize.Y + CurrentGap + BottomSlot->GetSize().Y * 0.5f;
	const float LeftDistance =
		DotHalfSize.X + CurrentGap + LeftSlot->GetSize().X * 0.5f;
	const float RightDistance =
		DotHalfSize.X + CurrentGap + RightSlot->GetSize().X * 0.5f;

	CenterDot->SetRenderTranslation(FVector2D::ZeroVector);
	TopBar->SetRenderTranslation(FVector2D(0.0f, -TopDistance));
	BottomBar->SetRenderTranslation(FVector2D(0.0f, BottomDistance));
	LeftBar->SetRenderTranslation(FVector2D(-LeftDistance, 0.0f));
	RightBar->SetRenderTranslation(FVector2D(RightDistance, 0.0f));


}




