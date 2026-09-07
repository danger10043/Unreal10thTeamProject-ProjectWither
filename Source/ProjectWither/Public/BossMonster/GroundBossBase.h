// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterCharacterBase.h"
#include "GroundBossBase.generated.h"

/**
 * 
 */
class UBossComponent;

UCLASS()
class PROJECTWITHER_API AGroundBossBase : public AMonsterCharacterBase
{
    GENERATED_BODY()

public:
    AGroundBossBase();

    UFUNCTION(BlueprintPure, Category = "Boss")
    UBossComponent* GetBossComponent() const { return BossComponent; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    TObjectPtr<UBossComponent> BossComponent;
};