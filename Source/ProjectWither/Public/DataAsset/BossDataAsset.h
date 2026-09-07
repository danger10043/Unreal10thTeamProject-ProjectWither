#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/BossPhaseSettings.h"
#include "BossDataAsset.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class PROJECTWITHER_API UBossDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
    FText BossName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    float Phase2HealthRatio = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    FBossPhaseSettings Phase1Settings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    FBossPhaseSettings Phase2Settings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    TObjectPtr<UAnimMontage> PhaseTransitionMontage;
};