// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterAttack.generated.h"

class UMonsterComponent;
class UBehaviorTreeComponent;

UCLASS()
class PROJECTWITHER_API UBTTask_MonsterAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UFUNCTION()
	void HandleAttackFinished(bool bInterrupted);	// 공격 종료 이벤트를 받는 함수

protected:
	UPROPERTY()
	TObjectPtr<UMonsterComponent> CachedMonsterComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;

};
