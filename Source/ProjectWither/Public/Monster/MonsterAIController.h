// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "MonsterAIController.generated.h"

class UAIPerceptionComponent;
class UMonsterComponent;
class UStatComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

/**
 *
 */
UCLASS()
class PROJECTWITHER_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMonsterAIController();

	virtual void OnPossess(APawn* InPawn) override;

	void SetMonsterComponent(APawn* InPawn);
	void SetTargetActor(AActor* NewTarget);
	void ClearTargetActor();
	void StopAI();
	void RestartAI();

	bool IsValidTarget(AActor* InActor);

protected:
	UFUNCTION()
	void OnTargetDied();

	void ForgetTargetAfterSightLoss();

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UMonsterComponent> MonsterComponent;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> TargetStat = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception", meta = (ClampMin = "0.0"))
	float TargetForgetDelay = 2.0f;

	FTimerHandle TargetForgetTimerHandle;
	bool bCurrentTargetSensed = false;
};
