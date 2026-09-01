// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_MonsterAttack.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = TEXT("Monster Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UMonsterComponent* MonsterComponent = ControlledPawn->FindComponentByClass<UMonsterComponent>();

	if (!IsValid(MonsterComponent))
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedMonsterComponent = MonsterComponent;

	MonsterComponent->OnMonsterAttackFinished.AddUniqueDynamic(
		this,
		&UBTTask_MonsterAttack::HandleAttackFinished
	);

	if (!MonsterComponent->Attack())
	{
		MonsterComponent->OnMonsterAttackFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterAttack::HandleAttackFinished
		);

		CachedOwnerComp = nullptr;
		CachedMonsterComponent = nullptr;

		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_MonsterAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (IsValid(CachedMonsterComponent))
	{
		CachedMonsterComponent->OnMonsterAttackFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterAttack::HandleAttackFinished
		);
	}

	CachedOwnerComp = nullptr;
	CachedMonsterComponent = nullptr;

	return EBTNodeResult::Aborted;
}

void UBTTask_MonsterAttack::HandleAttackFinished(bool bInterrupted)
{
	UMonsterComponent* MonsterComponent = CachedMonsterComponent.Get();
	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();

	if (IsValid(MonsterComponent))
	{
		MonsterComponent->OnMonsterAttackFinished.RemoveDynamic(
			this,
			&UBTTask_MonsterAttack::HandleAttackFinished
		);
	}

	CachedOwnerComp = nullptr;
	CachedMonsterComponent = nullptr;

	if (!IsValid(OwnerComp))
	{
		return;
	}

	FinishLatentTask(*OwnerComp, 
		bInterrupted ? 
		EBTNodeResult::Failed : EBTNodeResult::Succeeded
	);
}
