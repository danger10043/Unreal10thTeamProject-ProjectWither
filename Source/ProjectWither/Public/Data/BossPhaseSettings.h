#pragma once

#include "CoreMinimal.h"
#include "BossPhaseSettings.generated.h"

USTRUCT(BlueprintType)
struct FBossPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MoveSpeed = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackCooldown = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackPowerMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DefenseMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FLinearColor BodyTint = FLinearColor::White;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FLinearColor FurTint = FLinearColor::White;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float EmissiveStrength = 0.0f;
};