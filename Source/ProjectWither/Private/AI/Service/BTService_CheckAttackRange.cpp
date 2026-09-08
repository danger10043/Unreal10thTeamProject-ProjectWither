// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_CheckAttackRange.h"
#include "CommonHeader/MonsterStateEnums.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = TEXT("Check Attack Range");
	Interval = 0.1f;
	RandomDeviation = 0.0f;

	IsInAttackRangeKey.AddBoolFilter(
		this,
		GET_MEMBER_NAME_CHECKED(
			UBTService_CheckAttackRange,
			IsInAttackRangeKey));
}

void UBTService_CheckAttackRange::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = IsValid(AIController) ? AIController->GetPawn() : nullptr;

	UMonsterComponent* MonsterComponent = IsValid(ControlledPawn)
		? ControlledPawn->FindComponentByClass<UMonsterComponent>()
		: nullptr;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(MonsterComponent) || !IsValid(Blackboard))
	{
		return;
	}

	Blackboard->SetValueAsBool(
		IsInAttackRangeKey.SelectedKeyName,
		MonsterComponent->IsInAttackRange());
}
