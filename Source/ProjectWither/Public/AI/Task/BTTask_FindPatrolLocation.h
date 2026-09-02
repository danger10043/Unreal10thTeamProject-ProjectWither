// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_FindPatrolLocation.generated.h"

UCLASS()
class PROJECTWITHER_API UBTTask_FindPatrolLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindPatrolLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FBlackboardKeySelector PatrolLocationKey;

    UPROPERTY(EditAnywhere, Category = "Patrol",
        meta = (ClampMin = "0.0"))
    float PatrolRadius = 700.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FVector NavProjectionExtent = FVector(100.0f, 100.0f, 300.0f);
};