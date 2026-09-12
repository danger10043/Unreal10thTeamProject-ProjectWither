#include "Widget/TestMainUIWidget.h"

#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "DataAsset/WeaponDataAsset.h"
#include "Widget/AmmoCountWidget.h"
#include "Widget/WeaponTypeWidget.h"
#include "Widget/PotionCountWidget.h"
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

	WeaponComponent = OwningPawn->FindComponentByClass<UWeaponComponent>();

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.AddUniqueDynamic(
			this,
			&UTestMainUIWidget::HandleWeaponChanged
		);
	}

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
	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.RemoveDynamic(
			this,
			&UTestMainUIWidget::HandleWeaponChanged
		);
	}

	WeaponComponent = nullptr;

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
	HandleWeaponChanged();

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

void UTestMainUIWidget::HandleWeaponChanged()
{
	if (IsValid(WeaponTypeWidget))
	{
		const EWeaponType CurrentWeaponType =
			IsValid(WeaponComponent)
			? WeaponComponent->GetWeaponType()
			: EWeaponType::None;

		WeaponTypeWidget->SetWeaponType(CurrentWeaponType);
	}

	if (!IsValid(AmmoCountWidget)) return;

	if (!IsValid(WeaponComponent) || !WeaponComponent->IsGunEquipped())
	{
		AmmoCountWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const UWeaponDataAsset* WeaponData =
		WeaponComponent->GetCurrentWeaponData();

	if (!IsValid(WeaponData))
	{
		AmmoCountWidget->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	AmmoCountWidget->SetAmmo(
		WeaponComponent->GetCurrentAmmo(),
		WeaponData->GetMaxAmmo()
	);

	AmmoCountWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
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


