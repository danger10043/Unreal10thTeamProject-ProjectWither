// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/EnhancementDataAsset.h"

bool UEnhancementDataAsset::GetCostForTargetLevel(int32 TargetLevel, FEnhancementLevelCost& OutCost) const
{
	const FEnhancementLevelCost* FoundCost = FindCostForTargetLevel(TargetLevel);

	if (FoundCost == nullptr)
	{
		OutCost = FEnhancementLevelCost();
		return false;
	}

	OutCost = *FoundCost;
	return true;
}

const FEnhancementLevelCost* UEnhancementDataAsset::FindCostForTargetLevel(int32 TargetLevel) const
{
	if (TargetLevel < 1 || TargetLevel > MaxEnhanceLevel)
	{
		return nullptr;
	}

	return LevelCosts.FindByPredicate([TargetLevel](const FEnhancementLevelCost& Cost)
		{
			return Cost.TargetLevel == TargetLevel;
		});
}
