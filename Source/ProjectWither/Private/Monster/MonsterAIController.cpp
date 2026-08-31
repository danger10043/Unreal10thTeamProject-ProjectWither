// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/MonsterAIController.h"
#include "Component/MonsterComponent.h"
#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"

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
	if (IsValidTarget(InPawn))
	{
		SetMonsterComponent(InPawn);
	}
	if (BehaviorTree)
	{
		UBlackboardComponent* BlackboardComp = nullptr;
		UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
		RunBehaviorTree(BehaviorTree);
	}
}

void AMonsterAIController::SetMonsterComponent(APawn* InPawn)
{
	MonsterComponent = InPawn->FindComponentByClass<UMonsterComponent>();
}

void AMonsterAIController::SetTargetActor(AActor* NewTarget)
{
	if (!IsValid(MonsterComponent) || MonsterComponent->IsDead())
	{
		return;
	}

	if (!IsValidTarget(NewTarget))
	{
		return;
	}

	MonsterComponent->SetTarget(NewTarget);

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TEXT("TargetActor"), NewTarget);
		BB->SetValueAsBool(TEXT("bHasTarget"), true);
	}
}

void AMonsterAIController::ClearTargetActor()
{
	if (IsValid(MonsterComponent))
	{
		MonsterComponent->ClearTarget();
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(TEXT("TargetActor"));
		BB->SetValueAsBool(TEXT("bHasTarget"), false);
	}
}

void AMonsterAIController::StopAI()
{
}

bool AMonsterAIController::IsValidTarget(AActor* InActor)
{
	// 파괴되었거나 없는 액터는 제외
	if (!IsValid(InActor))
	{
		return false;
	}

	// 자기 자신은 제외
	if (InActor == GetPawn())
	{
		return false;
	}

	// 플레이어 캐릭터만 공격 대상으로 허용
	APlayerCharacter* Player = Cast<APlayerCharacter>(InActor);
	if (!IsValid(Player))
	{
		return false;
	}

	// 스탯 인터페이스를 통해 플레이어 체력 확인
	UStatComponent* TargetStat =
		IStatComponentUserInterface::Execute_GetStatComponent(Player);

	return IsValid(TargetStat) && !TargetStat->IsHealthZero();
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* InActor, FAIStimulus Stimulus)
{
	if (!IsValid(MonsterComponent) || MonsterComponent->IsDead())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Perception: %s, Sensed: %d"), *GetNameSafe(InActor), Stimulus.WasSuccessfullySensed());

	if (Stimulus.WasSuccessfullySensed())
	{
		SetTargetActor(InActor);
	}
	else
	{
		if (InActor == MonsterComponent->GetTargetActor())
		{
			ClearTargetActor();
		}
	}
}