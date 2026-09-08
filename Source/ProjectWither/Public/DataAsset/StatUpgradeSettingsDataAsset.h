#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/StatUpgradeTypes.h"
#include "StatUpgradeSettingsDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTWITHER_API UStatUpgradeSettingsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UStatUpgradeSettingsDataAsset();

	const FStatUpgradeRuleSet& GetRules() const { return Rules; }

	const FStatUpgradeRule* FindRule(EUpgradeableStatType StatType) const;

	UFUNCTION(BlueprintPure, Category = "Stat Upgrade")
	bool GetUpgradeRule(EUpgradeableStatType StatType, FStatUpgradeRule& OutRule) const;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	FStatUpgradeRuleSet Rules;
};
