// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_FindFlightPatrolLocation.generated.h"

// 네브메쉬를 사용하지 않는 비행 몬스터용 순찰 지점 탐색 태스크.
// 스폰 위치를 중심으로 수평 반경 + 고도 범위 안에서 랜덤 지점을 뽑고,
// 장애물에 막히지 않은 지점을 찾을 때까지 재시도한다.
UCLASS()
class PROJECTWITHER_API UBTTask_FindFlightPatrolLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FindFlightPatrolLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FBlackboardKeySelector PatrolLocationKey;

    // 스폰 위치 기준 수평(XY) 순찰 반경
    UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "0.0"))
    float PatrolRadius = 1000.0f;

    // 스폰 위치 기준 최소/최대 고도 오프셋 (Z)
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MinFlightHeight = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MaxFlightHeight = 600.0f;

    // 후보 지점이 비어있는지 검사할 구체 반경
    UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "0.0"))
    float ClearanceRadius = 60.0f;

    // 빈 공간을 찾기 위한 최대 재시도 횟수
    UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "1"))
    int32 MaxSampleAttempts = 8;
};
