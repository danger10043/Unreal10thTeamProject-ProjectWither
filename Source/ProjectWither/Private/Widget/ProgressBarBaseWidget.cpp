#include "Widget/ProgressBarBaseWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UProgressBarBaseWidget::SetValues(float CurrentValue, float MaxValue)
{
	const float SafeMaxValue = FMath::Max(0.0f, MaxValue);
	const float SafeCurrentValue = FMath::Clamp(CurrentValue, 0.0f, SafeMaxValue);

	if (IsValid(TargetProgressBar))
	{
		const float Percent = SafeMaxValue > UE_SMALL_NUMBER ?
			SafeCurrentValue / SafeMaxValue
			: 0.0f;

		TargetProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}

	if (IsValid(CurrentValueText))
	{
		CurrentValueText->SetText(
			FText::AsNumber(FMath::RoundToInt(SafeCurrentValue))
		);
	}

	if (IsValid(MaxValueText))
	{
		MaxValueText->SetText(
			FText::AsNumber(FMath::RoundToInt(SafeMaxValue))
		);
	}
}

void UProgressBarBaseWidget::SetBarColors(FLinearColor BackgroundColor, FLinearColor FillColor)
{
	if (!IsValid(TargetProgressBar)) return;

	FProgressBarStyle ProgressBarStyle = TargetProgressBar->GetWidgetStyle();

	ProgressBarStyle.BackgroundImage.TintColor = FSlateColor(BackgroundColor);

	TargetProgressBar->SetWidgetStyle(ProgressBarStyle);
	TargetProgressBar->SetFillColorAndOpacity(FillColor);
}
