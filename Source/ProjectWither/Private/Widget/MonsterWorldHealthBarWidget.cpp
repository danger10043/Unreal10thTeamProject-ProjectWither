#include "Widget/MonsterWorldHealthBarWidget.h"

#include "Components/ProgressBar.h"

void UMonsterWorldHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
	TargetHealthPercent = MaxHealth > UE_SMALL_NUMBER
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	if (!bHealthInitialized)
	{
		DisplayedHealthPercent = TargetHealthPercent;
		bHealthInitialized = true;
	}

	if (IsValid(HealthProgressBar))
	{
		HealthProgressBar->SetPercent(DisplayedHealthPercent);
	}
}

void UMonsterWorldHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bHealthInitialized || !IsValid(HealthProgressBar)) return;

	DisplayedHealthPercent = HealthInterpolationSpeed <= 0.0f
		? TargetHealthPercent
		: FMath::FInterpTo(
			DisplayedHealthPercent, TargetHealthPercent, InDeltaTime, HealthInterpolationSpeed);
	if (FMath::IsNearlyEqual(DisplayedHealthPercent, TargetHealthPercent, 0.001f))
	{
		DisplayedHealthPercent = TargetHealthPercent;
	}
	HealthProgressBar->SetPercent(DisplayedHealthPercent);
}
