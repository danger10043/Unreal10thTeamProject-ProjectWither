#include "Widget/StatBarSegmentWidget.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"

void UStatBarSegmentWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyAppearance();
}

void UStatBarSegmentWidget::SetAppearance(const FSlateBrush& Brush, FVector2D Size)
{
	CurrentBrush = Brush;
	CurrentSize.X = FMath::Max(1.0f, Size.X);
	CurrentSize.Y = FMath::Max(1.0f, Size.Y);
	bHasAppearance = true;

	ApplyAppearance();
}

void UStatBarSegmentWidget::ApplyAppearance()
{
	if (IsValid(SegmentSizeBox))
	{
		SegmentSizeBox->SetWidthOverride(CurrentSize.X);
		SegmentSizeBox->SetHeightOverride(CurrentSize.Y);
	}

	if (bHasAppearance && IsValid(SegmentImage))
	{
		SegmentImage->SetBrush(CurrentBrush);
	}
}