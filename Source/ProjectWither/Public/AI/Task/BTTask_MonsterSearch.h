// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterSearch.generated.h"

/**
 * 
 */
class UMonsterComponent;
class UBehaviorTreeComponent;

UCLASS()
class PROJECTWITHER_API UBTTask_MonsterSearch : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterSearch();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UFUNCTION()
	void HandleSearchFinished(bool bInterrupted);

private:
	UPROPERTY()
	TObjectPtr<UMonsterComponent> CachedMonsterComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;
};
