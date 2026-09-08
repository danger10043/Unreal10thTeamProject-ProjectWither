// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckAttackRange.generated.h"

// 공격 사거리 판정을 매 프레임(Interval) 계산해 블랙보드 Bool 키에 반영.
// CalculateRawConditionValue로 직접 계산하는 커스텀 데코레이터는 값이 바뀌어도
// 자동으로 재평가되지 않으므로, 이 서비스로 블랙보드 값을 갱신하고
// 표준 Blackboard 데코레이터로 감시해야 Observer Aborts가 실시간으로 동작한다.
UCLASS()
class PROJECTWITHER_API UBTService_CheckAttackRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckAttackRange();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInAttackRangeKey;
};
