#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_Wait.h"
#include "BTTask_MonsterWait.generated.h"

UCLASS()
class PROJECTWITHER_API UBTTask_MonsterWait : public UBTTask_Wait
{
	GENERATED_BODY()

public:
	UBTTask_MonsterWait();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};
