#include "AI/Task/BTTask_MonsterMoveTo.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_MonsterMoveTo::UBTTask_MonsterMoveTo()
{
	NodeName = TEXT("Monster Move To (Allow Range)");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MonsterMoveTo::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = IsValid(AIController) ? AIController->GetPawn() : nullptr;
	const UMonsterComponent* Monster = IsValid(Pawn)
		? Pawn->FindComponentByClass<UMonsterComponent>()
		: nullptr;

	if (!IsValid(Monster))
	{
		return EBTNodeResult::Failed;
	}

	AcceptableRadius = Monster->GetAllowRange();
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
