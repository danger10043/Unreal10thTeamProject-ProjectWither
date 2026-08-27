// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMonsterAIController::AMonsterAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 800.f;
	SightConfig->LoseSightRadius = 900.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree)
	{
		UBlackboardComponent* BlackboardComp = nullptr;
		UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
		RunBehaviorTree(BehaviorTree);
	}
}

void AMonsterAIController::SetTargetActor(AActor* NewTarget)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TEXT("TargetActor"), NewTarget);
		BB->SetValueAsBool(TEXT("bHasTarget"), NewTarget != nullptr);
	}
}

void AMonsterAIController::ClearTargetActor()
{
	SetTargetActor(nullptr);
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UE_LOG(LogTemp, Warning, TEXT("Perception: %s, Sensed: %d"), *GetNameSafe(Actor), Stimulus.WasSuccessfullySensed());

	if (Stimulus.WasSuccessfullySensed())
	{
		SetTargetActor(Actor);
	}
	else
	{
		ClearTargetActor();
	}
}