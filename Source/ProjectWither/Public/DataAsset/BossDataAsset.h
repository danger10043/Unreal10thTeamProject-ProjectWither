#pragma once

#include "CoreMinimal.h"
#include "DataAsset/MonsterDataAsset.h"
#include "BossDataAsset.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class PROJECTWITHER_API UBossDataAsset : public UMonsterDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    float Phase2HealthRatio = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    FBossPhaseSettings Phase2Settings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phase")
    TObjectPtr<UAnimMontage> PhaseTransitionMontage;
};
