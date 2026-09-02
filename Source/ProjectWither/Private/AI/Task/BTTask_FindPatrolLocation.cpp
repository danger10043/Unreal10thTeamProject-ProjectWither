// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_FindPatrolLocation.h"
#include "CommonHeader/MonsterStateEnums.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
    NodeName = TEXT("Find Patrol Location");

    PatrolLocationKey.AddVectorFilter(
        this,
        GET_MEMBER_NAME_CHECKED(
            UBTTask_FindPatrolLocation,
            PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(
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

    UNavigationSystemV1* NavigationSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(
            ControlledPawn->GetWorld());

    if (!IsValid(Blackboard) ||
        !IsValid(MonsterComponent) ||
        !IsValid(NavigationSystem))
    {
        return EBTNodeResult::Failed;
    }

    // 풀에서 다시 소환된 경우까지 반영된 시작 위치
    const FVector SpawnLocation =
        MonsterComponent->GetSpawnLocation();

    // 시작 위치가 NavMesh에서 조금 벗어나 있어도
    // 가까운 NavMesh 위치로 보정
    FNavLocation ProjectedOrigin;

    if (!NavigationSystem->ProjectPointToNavigation(
        SpawnLocation,
        ProjectedOrigin,
        NavProjectionExtent))
    {
        return EBTNodeResult::Failed;
    }

    // 시작 위치와 연결되어 실제로 도달할 수 있는 점을 선택
    FNavLocation PatrolLocation;

    if (!NavigationSystem->GetRandomReachablePointInRadius(
        ProjectedOrigin.Location,
        PatrolRadius,
        PatrolLocation))
    {
        return EBTNodeResult::Failed;
    }

    Blackboard->SetValueAsVector(
        PatrolLocationKey.SelectedKeyName,
        PatrolLocation.Location);

    MonsterComponent->SetMonsterState(
        EMonsterState::Patrol);

    return EBTNodeResult::Succeeded;
}