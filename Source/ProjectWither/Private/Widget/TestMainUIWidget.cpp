#include "Widget/TestMainUIWidget.h"

#include "Component/CombatComponent.h"
#include "Component/StatComponent.h"
#include "GameFramework/Pawn.h"
#include "Interface/CombatComponentUserInterface.h"
#include "Interface/StatComponentUserInterface.h"
#include "Widget/CurrentStateWidget.h"
#include "Widget/ProgressBarBaseWidget.h"

void UTestMainUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerComponents();
	InitializeWidgetColors();
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

	if (OwningPawn->GetClass()->ImplementsInterface(
		UCombatComponentUserInterface::StaticClass()
	))
	{
		CombatComponent = ICombatComponentUserInterface::Execute_GetCombatComponent(OwningPawn);
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

	if (IsValid(CombatComponent))
	{
		CombatComponent->OnActionStateChangedEvent.AddUniqueDynamic(
			this,
			&UTestMainUIWidget::HandleActionStateChanged
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

	if (IsValid(CombatComponent))
	{
		CombatComponent->OnActionStateChangedEvent.RemoveDynamic(
			this,
			&UTestMainUIWidget::HandleActionStateChanged
		);
	}

	StatComponent = nullptr;
	CombatComponent = nullptr;
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

	if (IsValid(CombatComponent))
	{
		HandleActionStateChanged(
			EPlayerActionState::None,
			CombatComponent->GetActionState()
		);
	}
	else if (IsValid(CurrentStateWidget))
	{
		CurrentStateWidget->SetPlayerActionState(EPlayerActionState::None);
	}
}

void UTestMainUIWidget::InitializeWidgetColors()
{
	if (IsValid(HealthBarWidget))
	{
		HealthBarWidget->SetBarColors(
			FLinearColor(0.4f, 0.0f, 0.0f, 0.35f),
			FLinearColor(0.9f, 0.05f, 0.05f, 1.0f)
		);
	}

	if (IsValid(StaminaBarWidget))
	{
		StaminaBarWidget->SetBarColors(
			FLinearColor(0.0f, 0.1f, 0.4f, 0.35f),
			FLinearColor(0.05f, 0.25f, 1.0f, 1.0f)
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

void UTestMainUIWidget::HandleActionStateChanged(EPlayerActionState PreviousState, EPlayerActionState NewState)
{
	if (IsValid(CurrentStateWidget))
	{
		CurrentStateWidget->SetPlayerActionState(NewState);
	}
}


