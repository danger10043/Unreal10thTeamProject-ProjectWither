// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_MonsterTurnToTarget.generated.h"

/**
 * 
 */
class APawn;
class UAnimInstance;
class UAnimMontage;
class UBehaviorTreeComponent;
class UPawnMovementComponent;

UCLASS()
class PROJECTWITHER_API UBTTask_MonsterTurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterTurnToTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

private:
	void FinishTurn(bool bSucceeded);
	void RestoreMovement();

	UFUNCTION()
	void OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Turn")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Turn")
	TObjectPtr<UAnimMontage> TurnMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Turn",
		meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MinimumTurnAngle = 30.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<APawn> CachedPawn = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> CachedAnimInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPawnMovementComponent> CachedMovement = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp = nullptr;

	FRotator StartRotation;
	FRotator TargetRotation;

	float DeltaYaw = 0.0f;
	float ElapsedTime = 0.0f;
	float TurnDuration = 0.0f;
	bool bMovementWasActive = false;
};
