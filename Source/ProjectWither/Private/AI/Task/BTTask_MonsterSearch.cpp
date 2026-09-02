// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_MonsterSearch.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_MonsterSearch::UBTTask_MonsterSearch()
{
	NodeName = TEXT("Monster Search");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MonsterSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	APawn* Pawn = IsValid(AIController) ?
		AIController->GetPawn() : nullptr;

	UMonsterComponent* MonsterComponent = IsValid(Pawn) ?
		Pawn->FindComponentByClass<UMonsterComponent>() : nullptr;

	if (!IsValid(MonsterComponent))
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedMonsterComponent = MonsterComponent;

	MonsterComponent->OnMonsterSearchFinished.AddUniqueDynamic(
		this,
		&UBTTask_MonsterSearch::HandleSearchFinished
	);

	if (!MonsterComponent->PlaySearchAnimation())
	{
		MonsterComponent->OnMonsterSearchFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterSearch::HandleSearchFinished
		);

		CachedOwnerComp = nullptr;
		CachedMonsterComponent = nullptr;

		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_MonsterSearch::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (IsValid(CachedMonsterComponent))
	{
		CachedMonsterComponent->OnMonsterSearchFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterSearch::HandleSearchFinished
		);

		CachedMonsterComponent->CancelSearch();
	}

	CachedOwnerComp = nullptr;
	CachedMonsterComponent = nullptr;

	return EBTNodeResult::Aborted;
}

void UBTTask_MonsterSearch::HandleSearchFinished(bool bInterrupted)
{
	UMonsterComponent* MonsterComponent = CachedMonsterComponent.Get();

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();

	if (IsValid(MonsterComponent))
	{
		CachedMonsterComponent->OnMonsterSearchFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterSearch::HandleSearchFinished
		);

	}
	CachedOwnerComp = nullptr;
	CachedMonsterComponent = nullptr;

	if (!IsValid(OwnerComp))
	{
		return;
	}

	FinishLatentTask(
		*OwnerComp,
		bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded
	);
}
