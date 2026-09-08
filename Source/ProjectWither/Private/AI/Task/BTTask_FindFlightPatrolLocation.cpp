// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindFlightPatrolLocation.h"
#include "CommonHeader/MonsterStateEnums.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Component/MonsterComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UBTTask_FindFlightPatrolLocation::UBTTask_FindFlightPatrolLocation()
{
    NodeName = TEXT("Find Flight Patrol Location");

    PatrolLocationKey.AddVectorFilter(
        this,
        GET_MEMBER_NAME_CHECKED(
            UBTTask_FindFlightPatrolLocation,
            PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_FindFlightPatrolLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!IsValid(ControlledPawn))
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* Blackboard =
        OwnerComp.GetBlackboardComponent();

    UMonsterComponent* MonsterComponent =
        ControlledPawn->FindComponentByClass<UMonsterComponent>();

    UWorld* World = ControlledPawn->GetWorld();

    if (!IsValid(Blackboard) ||
        !IsValid(MonsterComponent) ||
        !World)
    {
        return EBTNodeResult::Failed;
    }

    // 풀에서 다시 소환된 경우까지 반영된 시작 위치
    const FVector SpawnLocation =
        MonsterComponent->GetSpawnLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(ControlledPawn);

    bool bFoundClearLocation = false;
    FVector PatrolLocation = SpawnLocation;

    // 네브메쉬 없이 스폰 위치 주변 원기둥 범위에서 빈 공간을 찾을 때까지 샘플링
    for (int32 Attempt = 0; Attempt < MaxSampleAttempts; ++Attempt)
    {
        const float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
        const float RandomRadius = FMath::FRandRange(0.0f, PatrolRadius);
        const float RandomHeight = FMath::FRandRange(MinFlightHeight, MaxFlightHeight);

        const FVector CandidateLocation = SpawnLocation + FVector(
            FMath::Cos(RandomAngle) * RandomRadius,
            FMath::Sin(RandomAngle) * RandomRadius,
            RandomHeight);

        const bool bBlocked = World->OverlapBlockingTestByChannel(
            CandidateLocation,
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeSphere(ClearanceRadius),
            QueryParams);

        if (!bBlocked)
        {
            PatrolLocation = CandidateLocation;
            bFoundClearLocation = true;
            break;
        }
    }

    if (!bFoundClearLocation)
    {
        // 빈 공간을 못 찾아도 실패시키지 않고 스폰 위치 바로 위로 폴백.
        // 실패를 반환하면 다음 틱에 딜레이 없이 즉시 재시도되어 지형이
        // 복잡한 맵에서 매 프레임 재시도하며 깜빡이는 현상이 생김
        PatrolLocation = SpawnLocation +
            FVector(0.0f, 0.0f, (MinFlightHeight + MaxFlightHeight) * 0.5f);
    }

    Blackboard->SetValueAsVector(
        PatrolLocationKey.SelectedKeyName,
        PatrolLocation);

    MonsterComponent->SetMonsterState(
        EMonsterState::Patrol);

    return EBTNodeResult::Succeeded;
}
