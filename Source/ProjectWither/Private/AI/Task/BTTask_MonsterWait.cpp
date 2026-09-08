#include "AI/Task/BTTask_MonsterWait.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Component/MonsterComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_MonsterWait::UBTTask_MonsterWait()
{
	NodeName = TEXT("Monster Wait (Attack Cooldown)");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_MonsterWait::ExecuteTask(
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

	WaitTime = Monster->GetAttackCooldown();
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
