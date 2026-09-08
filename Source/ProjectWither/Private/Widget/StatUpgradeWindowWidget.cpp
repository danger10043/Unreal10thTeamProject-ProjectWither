#include "Widget/StatUpgradeWindowWidget.h"

#include "Component/InventoryComponent.h"
#include "Component/StatComponent.h"
#include "Component/StatUpgradeComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

namespace
{
	FText FormatCostText(const FStatUpgradeCost& Cost)
	{
		return FText::FromString(FString::Printf(TEXT("%dG"), Cost.Gold));
	}

	FText FormatResultText(EStatUpgradeResult Result)
	{
		switch (Result)
		{
		case EStatUpgradeResult::Success:
			return FText::FromString(TEXT("강화 성공"));
		case EStatUpgradeResult::MissingStatComponent:
			return FText::FromString(TEXT("스탯 컴포넌트가 없습니다."));
		case EStatUpgradeResult::MissingInventoryComponent:
			return FText::FromString(TEXT("인벤토리 컴포넌트가 없습니다."));
		case EStatUpgradeResult::InvalidUpgradeSettings:
			return FText::FromString(TEXT("강화 설정이 올바르지 않습니다."));
		case EStatUpgradeResult::MaxLevelReached:
			return FText::FromString(TEXT("이미 최대 단계입니다."));
		case EStatUpgradeResult::NotEnoughGold:
			return FText::FromString(TEXT("골드가 부족합니다."));
		default:
			return FText::GetEmpty();
		}
	}
}

void UStatUpgradeWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindPlayerComponents();
	BindButtons();
	RefreshUpgradeInfo();
}

void UStatUpgradeWindowWidget::NativeDestruct()
{
	UnbindButtons();
	UnbindPlayerComponents();

	Super::NativeDestruct();
}

void UStatUpgradeWindowWidget::RefreshUpgradeInfo()
{
	RefreshOwnedCurrencyText();
	RefreshUpgradeRow(EUpgradeableStatType::Health, HealthLevelText, HealthCostText);
	RefreshUpgradeRow(EUpgradeableStatType::Stamina, StaminaLevelText, StaminaCostText);
	RefreshUpgradeRow(EUpgradeableStatType::AttackPower, AttackPowerLevelText, AttackPowerCostText);
	RefreshUpgradeRow(EUpgradeableStatType::DefensePower, DefensePowerLevelText, DefensePowerCostText);
}

EStatUpgradeResult UStatUpgradeWindowWidget::TryUpgradeHealth()
{
	return TryUpgrade(EUpgradeableStatType::Health);
}

EStatUpgradeResult UStatUpgradeWindowWidget::TryUpgradeStamina()
{
	return TryUpgrade(EUpgradeableStatType::Stamina);
}

EStatUpgradeResult UStatUpgradeWindowWidget::TryUpgradeAttackPower()
{
	return TryUpgrade(EUpgradeableStatType::AttackPower);
}

EStatUpgradeResult UStatUpgradeWindowWidget::TryUpgradeDefensePower()
{
	return TryUpgrade(EUpgradeableStatType::DefensePower);
}

void UStatUpgradeWindowWidget::BindPlayerComponents()
{
	APawn* OwningPawn = GetOwningPlayerPawn();

	if (!IsValid(OwningPawn))
	{
		return;
	}

	StatUpgradeComponent = OwningPawn->FindComponentByClass<UStatUpgradeComponent>();
	InventoryComponent = OwningPawn->FindComponentByClass<UInventoryComponent>();
	StatComponent = OwningPawn->FindComponentByClass<UStatComponent>();

	if (IsValid(StatUpgradeComponent))
	{
		StatUpgradeComponent->OnStatUpgradeLevelChanged.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleUpgradeLevelChanged);
	}

	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnGoldChanged.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleGoldChanged);
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnStatsChanged.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleStatsChanged);
	}
}

void UStatUpgradeWindowWidget::UnbindPlayerComponents()
{
	if (IsValid(StatUpgradeComponent))
	{
		StatUpgradeComponent->OnStatUpgradeLevelChanged.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleUpgradeLevelChanged);
	}

	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnGoldChanged.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleGoldChanged);
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnStatsChanged.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleStatsChanged);
	}

	StatUpgradeComponent = nullptr;
	InventoryComponent = nullptr;
	StatComponent = nullptr;
}

void UStatUpgradeWindowWidget::BindButtons()
{
	if (IsValid(HealthUpgradeButton))
	{
		HealthUpgradeButton->OnClicked.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleHealthUpgradeClicked);
	}

	if (IsValid(StaminaUpgradeButton))
	{
		StaminaUpgradeButton->OnClicked.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleStaminaUpgradeClicked);
	}

	if (IsValid(AttackPowerUpgradeButton))
	{
		AttackPowerUpgradeButton->OnClicked.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleAttackPowerUpgradeClicked);
	}

	if (IsValid(DefensePowerUpgradeButton))
	{
		DefensePowerUpgradeButton->OnClicked.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleDefensePowerUpgradeClicked);
	}

	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleCloseClicked);
	}
}

void UStatUpgradeWindowWidget::UnbindButtons()
{
	if (IsValid(HealthUpgradeButton))
	{
		HealthUpgradeButton->OnClicked.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleHealthUpgradeClicked);
	}

	if (IsValid(StaminaUpgradeButton))
	{
		StaminaUpgradeButton->OnClicked.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleStaminaUpgradeClicked);
	}

	if (IsValid(AttackPowerUpgradeButton))
	{
		AttackPowerUpgradeButton->OnClicked.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleAttackPowerUpgradeClicked);
	}

	if (IsValid(DefensePowerUpgradeButton))
	{
		DefensePowerUpgradeButton->OnClicked.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleDefensePowerUpgradeClicked);
	}

	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.RemoveDynamic(
			this,
			&UStatUpgradeWindowWidget::HandleCloseClicked);
	}
}

EStatUpgradeResult UStatUpgradeWindowWidget::TryUpgrade(EUpgradeableStatType StatType)
{
	if (!IsValid(StatUpgradeComponent))
	{
		SetResultText(EStatUpgradeResult::MissingStatComponent);
		return EStatUpgradeResult::MissingStatComponent;
	}

	const EStatUpgradeResult Result = StatUpgradeComponent->TryUpgradeStat(StatType);

	SetResultText(Result);
	RefreshUpgradeInfo();

	return Result;
}

void UStatUpgradeWindowWidget::RefreshUpgradeRow(
	EUpgradeableStatType StatType,
	UTextBlock* LevelText,
	UTextBlock* CostText) const
{
	if (!IsValid(StatUpgradeComponent))
	{
		if (IsValid(LevelText))
		{
			LevelText->SetText(FText::FromString(TEXT("Lv. -")));
		}

		if (IsValid(CostText))
		{
			CostText->SetText(FText::FromString(TEXT("-")));
		}

		return;
	}

	const int32 CurrentLevel = StatUpgradeComponent->GetUpgradeLevel(StatType);
	const int32 MaxLevel = StatUpgradeComponent->GetMaxUpgradeLevel(StatType);

	if (IsValid(LevelText))
	{
		LevelText->SetText(FText::FromString(FString::Printf(
			TEXT("Lv. %d / %d"),
			CurrentLevel,
			MaxLevel)));
	}

	if (!IsValid(CostText))
	{
		return;
	}

	FStatUpgradeCost Cost;

	if (!StatUpgradeComponent->GetNextUpgradeCost(StatType, Cost))
	{
		CostText->SetText(FText::FromString(TEXT("MAX")));
		return;
	}

	CostText->SetText(FormatCostText(Cost));
}

void UStatUpgradeWindowWidget::RefreshOwnedCurrencyText() const
{
	if (!IsValid(InventoryComponent))
	{
		if (IsValid(GoldText))
		{
			GoldText->SetText(FText::FromString(TEXT("Gold: -")));
		}

		return;
	}

	if (IsValid(GoldText))
	{
		GoldText->SetText(FText::FromString(FString::Printf(
			TEXT("Gold: %d"),
			InventoryComponent->GetGold())));
	}
}

void UStatUpgradeWindowWidget::SetResultText(EStatUpgradeResult Result)
{
	if (IsValid(ResultText))
	{
		ResultText->SetText(FormatResultText(Result));
	}
}

void UStatUpgradeWindowWidget::HandleHealthUpgradeClicked()
{
	TryUpgradeHealth();
}

void UStatUpgradeWindowWidget::HandleStaminaUpgradeClicked()
{
	TryUpgradeStamina();
}

void UStatUpgradeWindowWidget::HandleAttackPowerUpgradeClicked()
{
	TryUpgradeAttackPower();
}

void UStatUpgradeWindowWidget::HandleDefensePowerUpgradeClicked()
{
	TryUpgradeDefensePower();
}

void UStatUpgradeWindowWidget::HandleCloseClicked()
{
	RemoveFromParent();

	APlayerController* PlayerController = GetOwningPlayer();

	if (!IsValid(PlayerController))
	{
		return;
	}

	PlayerController->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
}

void UStatUpgradeWindowWidget::HandleUpgradeLevelChanged(EUpgradeableStatType StatType, int32 NewLevel)
{
	RefreshUpgradeInfo();
}

void UStatUpgradeWindowWidget::HandleGoldChanged(int32 CurrentGold, int32 ChangedAmount)
{
	RefreshUpgradeInfo();
}

void UStatUpgradeWindowWidget::HandleStatsChanged()
{
	RefreshUpgradeInfo();
}
