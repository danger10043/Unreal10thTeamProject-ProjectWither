#include "Widget/TestMainUIWidget.h"

#include "Component/StatComponent.h"
#include "GameFramework/Pawn.h"
#include "Interface/StatComponentUserInterface.h"
#include "Widget/SegmentedStatBarWidget.h"

void UTestMainUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerComponents();
	InitializeWidgetValues();
}

void UTestMainUIWidget::NativeDestruct()
{
	UnbindPlayerComponents();

	Super::NativeDestruct();
}

void UTestMainUIWidget::BindPlayerComponents()
{
	APawn* OwningPawn = GetOwningPlayerPawn();

	if (!IsValid(OwningPawn)) return;

	if (OwningPawn->GetClass()->ImplementsInterface(
		UStatComponentUserInterface::StaticClass()
	))
	{
		StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwningPawn);
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.AddUniqueDynamic(
			this,
			&UTestMainUIWidget::HandleHealthChanged
		);

		StatComponent->OnStaminaChanged.AddUniqueDynamic(
			this,
			&UTestMainUIWidget::HandleStaminaChanged
		);
	}
}

void UTestMainUIWidget::UnbindPlayerComponents()
{
	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.RemoveDynamic(
			this,
			&UTestMainUIWidget::HandleHealthChanged
		);

		StatComponent->OnStaminaChanged.RemoveDynamic(
			this,
			&UTestMainUIWidget::HandleStaminaChanged
		);
	}

	StatComponent = nullptr;
}

void UTestMainUIWidget::InitializeWidgetValues()
{
	if (IsValid(StatComponent))
	{
		HandleHealthChanged(
			StatComponent->GetCurrentHealth(),
			StatComponent->GetMaxHealth(),
			0.0f
		);
		
		HandleStaminaChanged(
			StatComponent->GetCurrentStamina(),
			StatComponent->GetMaxStamina(),
			0.0f
		);
	}
}

void UTestMainUIWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	if (IsValid(HealthBarWidget))
	{
		HealthBarWidget->SetValues(CurrentHealth, MaxHealth);
	}
}

void UTestMainUIWidget::HandleStaminaChanged(float CurrentStamina, float MaxStamina, float ChangedAmount)
{
	if (IsValid(StaminaBarWidget))
	{
		StaminaBarWidget->SetValues(CurrentStamina, MaxStamina);
	}
}


