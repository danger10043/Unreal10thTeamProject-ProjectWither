#include "Widget/MonsterWorldHealthBarWidget.h"

#include "Components/ProgressBar.h"

void UMonsterWorldHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
	if (!IsValid(HealthProgressBar))
	{
		return;
	}

	const float Percent = MaxHealth > UE_SMALL_NUMBER
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	HealthProgressBar->SetPercent(Percent);
}
