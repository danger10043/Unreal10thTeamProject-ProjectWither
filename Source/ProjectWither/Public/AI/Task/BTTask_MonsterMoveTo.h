#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MonsterMoveTo.generated.h"

UCLASS()
class PROJECTWITHER_API UBTTask_MonsterMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MonsterMoveTo();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};
