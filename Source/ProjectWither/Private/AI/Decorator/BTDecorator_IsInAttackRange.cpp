// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_IsInAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTDecorator_IsInAttackRange::UBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("Is In Attack Range");
}

bool UBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();

	if (!IsValid(AIController))
	{
		return false;
	}

	APawn* ControlledPawn = AIController->GetPawn();

	if (!IsValid(ControlledPawn))
	{
		return false;
	}

	UMonsterComponent* MonsterComponent = ControlledPawn->FindComponentByClass<UMonsterComponent>();

	if (!IsValid(MonsterComponent))
	{
		return false;
	}

	return MonsterComponent->IsInAttackRange();
}
