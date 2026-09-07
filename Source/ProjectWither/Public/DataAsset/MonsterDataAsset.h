#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CommonHeader/MonsterDespawnEnums.h"
#include "Data/BossPhaseSettings.h"
#include "MonsterDataAsset.generated.h"

class UAnimMontage;
class UDataTable;
class APickupItem;

UCLASS(BlueprintType)
class PROJECTWITHER_API UMonsterDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
    int32 MonsterId = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
    FText MonsterName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Stat", meta = (ClampMin = "0.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Stat", meta = (ClampMin = "0.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Stat", meta = (ClampMin = "0.0"))
    float MinAttackPower = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Stat", meta = (ClampMin = "0.0"))
    float MaxAttackPower = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Stat", meta = (ClampMin = "0.0"))
    float DefensePower = 0.0f;

    // General monsters use this directly; bosses use it as their phase-one settings.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Combat")
    FBossPhaseSettings BaseSettings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Montage")
    TObjectPtr<UAnimMontage> AttackMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Montage")
    TObjectPtr<UAnimMontage> HitReactMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Montage")
    TObjectPtr<UAnimMontage> ParriedMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Montage")
    TObjectPtr<UAnimMontage> DeathMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Montage")
    TObjectPtr<UAnimMontage> SearchMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Drop")
    TObjectPtr<UDataTable> ItemDropTable = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Drop")
    TSubclassOf<APickupItem> ItemPickupClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Death")
    EMonsterDespawnPolicy DespawnPolicy = EMonsterDespawnPolicy::Destroy;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Death", meta = (ClampMin = "0.0", Units = "s"))
    float DespawnDelay = 5.0f;
};
