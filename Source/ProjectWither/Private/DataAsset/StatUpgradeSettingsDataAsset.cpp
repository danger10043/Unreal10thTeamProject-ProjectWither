#include "DataAsset/StatUpgradeSettingsDataAsset.h"

UStatUpgradeSettingsDataAsset::UStatUpgradeSettingsDataAsset()
{
	Rules.ApplyDefaultValues();
}

const FStatUpgradeRule* UStatUpgradeSettingsDataAsset::FindRule(EUpgradeableStatType StatType) const
{
	return Rules.FindRule(StatType);
}

bool UStatUpgradeSettingsDataAsset::GetUpgradeRule(EUpgradeableStatType StatType, FStatUpgradeRule& OutRule) const
{
	const FStatUpgradeRule* Rule = FindRule(StatType);

	if (!Rule)
	{
		return false;
	}

	OutRule = *Rule;
	return true;
}
