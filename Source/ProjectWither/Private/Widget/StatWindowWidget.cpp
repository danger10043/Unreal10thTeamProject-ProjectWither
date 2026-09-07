#include "Widget/StatWindowWidget.h"

#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Components/TextBlock.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/Pawn.h"
#include "Interface/EquipmentComponentUserInterface.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Layout/Clipping.h"
#include "Styling/SlateTypes.h"

namespace
{
	int32 RoundStatValue(float Value)
	{
		return FMath::RoundToInt(FMath::Max(0.0f, Value));
	}

	FString FormatFormulaBonus(float Value)
	{
		const int32 RoundedValue = FMath::RoundToInt(Value);
		return RoundedValue >= 0
			? FString::Printf(TEXT("+%d"), RoundedValue)
			: FString::Printf(TEXT("%d"), RoundedValue);
	}

	void ConfigureStatTextBlock(UTextBlock* TextBlock)
	{
		if (!IsValid(TextBlock))
		{
			return;
		}

		TextBlock->SetAutoWrapText(false);
		TextBlock->SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
		TextBlock->SetClipping(EWidgetClipping::ClipToBounds);
	}

	FString FormatSingleStatValue(float BaseValue, float BonusValue)
	{
		const int32 RoundedBaseValue = RoundStatValue(BaseValue);
		const int32 RoundedFinalValue = RoundStatValue(BaseValue + BonusValue);

		return FString::Printf(
			TEXT("%d(%d%s)"),
			RoundedFinalValue,
			RoundedBaseValue,
			*FormatFormulaBonus(BonusValue));
	}

	FString FormatSingleStatLine(const TCHAR* Label, float BaseValue, float BonusValue)
	{
		return FString::Printf(
			TEXT("%s  %s"),
			Label,
			*FormatSingleStatValue(BaseValue, BonusValue));
	}

	FString FormatHealthStatValue(float MaxHealth, float CurrentHealth)
	{
		return FString::Printf(
			TEXT("%d/%d"),
			RoundStatValue(MaxHealth),
			RoundStatValue(CurrentHealth));
	}

	FString FormatHealthStatLine(const TCHAR* Label, float MaxHealth, float CurrentHealth)
	{
		return FString::Printf(
			TEXT("%s  %s"),
			Label,
			*FormatHealthStatValue(MaxHealth, CurrentHealth));
	}

	FString FormatRangeTotalStatValue(float BaseMinValue, float BaseMaxValue, float BonusValue)
	{
		const float NormalizedBaseMinValue = FMath::Min(BaseMinValue, BaseMaxValue);
		const float NormalizedBaseMaxValue = FMath::Max(BaseMinValue, BaseMaxValue);

		return FString::Printf(
			TEXT("%d~%d"),
			RoundStatValue(NormalizedBaseMinValue + BonusValue),
			RoundStatValue(NormalizedBaseMaxValue + BonusValue));
	}

	FString FormatRangeTotalStatLine(const TCHAR* Label, float BaseMinValue, float BaseMaxValue, float BonusValue)
	{
		return FString::Printf(
			TEXT("%s  %s"),
			Label,
			*FormatRangeTotalStatValue(BaseMinValue, BaseMaxValue, BonusValue));
	}

	float CalculateMiddleStatValue(float MinValue, float MaxValue)
	{
		const float NormalizedMinValue = FMath::Min(MinValue, MaxValue);
		const float NormalizedMaxValue = FMath::Max(MinValue, MaxValue);
		return (NormalizedMinValue + NormalizedMaxValue) * 0.5f;
	}
}

void UStatWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ConfigureStatTextBlock(HealthStatText);
	ConfigureStatTextBlock(AttackPowerStatText);
	ConfigureStatTextBlock(DamageStatText);
	ConfigureStatTextBlock(DefensePowerStatText);
	ConfigureStatTextBlock(HealthValueText);
	ConfigureStatTextBlock(AttackPowerValueText);
	ConfigureStatTextBlock(DamageValueText);
	ConfigureStatTextBlock(DefensePowerValueText);

	BindPlayerComponents();
	RefreshStats();
}

void UStatWindowWidget::NativeDestruct()
{
	UnbindPlayerComponents();

	Super::NativeDestruct();
}

void UStatWindowWidget::RefreshStats()
{
	if (!IsValid(StatComponent))
	{
		return;
	}

	const float WeaponAttackPowerBonus =
		IsValid(EquipmentComponent)
		? EquipmentComponent->GetWeaponAttackPowerBonus()
		: 0.0f;

	const float ArmorDefensePowerBonus =
		IsValid(EquipmentComponent)
		? EquipmentComponent->GetArmorDefensePowerBonus()
		: 0.0f;

	if (IsValid(HealthStatText))
	{
		HealthStatText->SetText(FText::FromString(FormatHealthStatLine(
			TEXT("체력"),
			StatComponent->GetMaxHealth(),
			StatComponent->GetCurrentHealth())));
	}

	if (IsValid(HealthValueText))
	{
		HealthValueText->SetText(FText::FromString(FormatHealthStatLine(
			TEXT("체력"),
			StatComponent->GetMaxHealth(),
			StatComponent->GetCurrentHealth())));
	}

	const float BaseAttackPower = CalculateMiddleStatValue(
		StatComponent->GetMinAttackPower(),
		StatComponent->GetMaxAttackPower());

	if (IsValid(AttackPowerStatText))
	{
		AttackPowerStatText->SetText(FText::FromString(FormatSingleStatLine(
			TEXT("공격력"),
			BaseAttackPower,
			WeaponAttackPowerBonus)));
	}

	if (IsValid(AttackPowerValueText))
	{
		AttackPowerValueText->SetText(FText::FromString(FormatSingleStatLine(
			TEXT("공격력"),
			BaseAttackPower,
			WeaponAttackPowerBonus)));
	}

	if (IsValid(DamageStatText))
	{
		DamageStatText->SetText(FText::FromString(FormatRangeTotalStatLine(
			TEXT("데미지"),
			StatComponent->GetMinAttackPower(),
			StatComponent->GetMaxAttackPower(),
			WeaponAttackPowerBonus)));
	}

	if (IsValid(DamageValueText))
	{
		DamageValueText->SetText(FText::FromString(FormatRangeTotalStatLine(
			TEXT("데미지"),
			StatComponent->GetMinAttackPower(),
			StatComponent->GetMaxAttackPower(),
			WeaponAttackPowerBonus)));
	}

	if (IsValid(DefensePowerStatText))
	{
		DefensePowerStatText->SetText(FText::FromString(FormatSingleStatLine(
			TEXT("방어력"),
			StatComponent->GetDefensePower(),
			ArmorDefensePowerBonus)));
	}

	if (IsValid(DefensePowerValueText))
	{
		DefensePowerValueText->SetText(FText::FromString(FormatSingleStatLine(
			TEXT("방어력"),
			StatComponent->GetDefensePower(),
			ArmorDefensePowerBonus)));
	}
}

void UStatWindowWidget::BindPlayerComponents()
{
	APawn* OwningPawn = GetOwningPlayerPawn();

	if (!IsValid(OwningPawn))
	{
		return;
	}

	if (OwningPawn->GetClass()->ImplementsInterface(UStatComponentUserInterface::StaticClass()))
	{
		StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwningPawn);
	}
	else
	{
		StatComponent = OwningPawn->FindComponentByClass<UStatComponent>();
	}

	if (OwningPawn->GetClass()->ImplementsInterface(UEquipmentComponentUserInterface::StaticClass()))
	{
		EquipmentComponent = IEquipmentComponentUserInterface::Execute_GetEquipmentComponent(OwningPawn);
	}
	else
	{
		EquipmentComponent = OwningPawn->FindComponentByClass<UEquipmentComponent>();
	}

	if (OwningPawn->GetClass()->ImplementsInterface(UWeaponComponentUserInterface::StaticClass()))
	{
		WeaponComponent = IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwningPawn);
	}
	else
	{
		WeaponComponent = OwningPawn->FindComponentByClass<UWeaponComponent>();
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.AddUniqueDynamic(
			this,
			&UStatWindowWidget::HandleHealthChanged);
	}

	if (IsValid(EquipmentComponent))
	{
		EquipmentComponent->OnEquipmentChanged.AddUniqueDynamic(
			this,
			&UStatWindowWidget::RefreshStats);
	}

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.AddUniqueDynamic(
			this,
			&UStatWindowWidget::RefreshStats);
	}
}

void UStatWindowWidget::UnbindPlayerComponents()
{
	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.RemoveDynamic(
			this,
			&UStatWindowWidget::HandleHealthChanged);
	}

	if (IsValid(EquipmentComponent))
	{
		EquipmentComponent->OnEquipmentChanged.RemoveDynamic(
			this,
			&UStatWindowWidget::RefreshStats);
	}

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->OnWeaponChanged.RemoveDynamic(
			this,
			&UStatWindowWidget::RefreshStats);
	}

	StatComponent = nullptr;
	EquipmentComponent = nullptr;
	WeaponComponent = nullptr;
}

void UStatWindowWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	RefreshStats();
}
