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
#include "Perception/AISense_Sight.h"
#include "BrainComponent.h"

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

    SetMonsterComponent(InPawn);

    if (!IsValid(MonsterComponent) || MonsterComponent->IsDead())
    {
        return;
    }

    if (IsValid(BehaviorTree))
    {
        UBlackboardComponent* BlackboardComp = nullptr;

        if (UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp))
        {
            RunBehaviorTree(BehaviorTree);
        }
    }
}

void AMonsterAIController::SetMonsterComponent(APawn* InPawn)
{
    MonsterComponent = IsValid(InPawn)
        ? InPawn->FindComponentByClass<UMonsterComponent>()
        : nullptr;
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

	UStatComponent* NewTargetStat =
		IStatComponentUserInterface::Execute_GetStatComponent(NewTarget);

	if (!IsValid(NewTargetStat))
	{
		return;
	}

	// 이전 타겟의 이벤트 구독 해제
	if (IsValid(TargetStat))
	{
		TargetStat->OnHealthZero.RemoveDynamic(
			this, &AMonsterAIController::OnTargetDied);
	}

	// 멤버 변수에 저장
	TargetStat = NewTargetStat;
	MonsterComponent->SetTarget(NewTarget);

	TargetStat->OnHealthZero.AddUniqueDynamic(
		this, &AMonsterAIController::OnTargetDied);

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TEXT("TargetActor"), NewTarget);
		BB->SetValueAsBool(TEXT("bHasTarget"), true);
	}
}

void AMonsterAIController::ClearTargetActor()
{
	// 타겟을 잡은 적이 없어도 안전하게 해제
	if (IsValid(TargetStat))
	{
		TargetStat->OnHealthZero.RemoveDynamic(
			this, &AMonsterAIController::OnTargetDied);
	}

	TargetStat = nullptr;

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
	// 사망 이후 새로운 시야 감지 중단
	if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
	}

	// 블랙보드 변경으로 다른 행동이 시작되지 않도록 먼저 중단
	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("Monster died"));
	}

	StopMovement();

	// 컴포넌트와 블랙보드 타겟 모두 제거
	ClearTargetActor();
}

void AMonsterAIController::RestartAI()
{
	APawn* ControlledPawn = GetPawn();
	if (!IsValid(ControlledPawn))
	{
		return;
	}

	SetMonsterComponent(ControlledPawn);
	ClearTargetActor();

	if (IsValid(AIPerceptionComponent))
	{
		AIPerceptionComponent->ForgetAll();
		AIPerceptionComponent->SetSenseEnabled(
			UAISense_Sight::StaticClass(), true);
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TEXT("SelfActor"), ControlledPawn);
		BB->SetValueAsVector(
			TEXT("SpawnLocation"),
			ControlledPawn->GetActorLocation());
	}

	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->RestartLogic();
	}
	else if (IsValid(BehaviorTree))
	{
		RunBehaviorTree(BehaviorTree);
	}
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
	UStatComponent* CandidateStat =
		IStatComponentUserInterface::Execute_GetStatComponent(Player);

	return IsValid(CandidateStat) && !CandidateStat->IsHealthZero();
}

void AMonsterAIController::OnTargetDied()
{
	StopMovement();
	ClearTargetActor();
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* InActor, FAIStimulus Stimulus)
{
	if (!IsValid(MonsterComponent) || MonsterComponent->IsDead())
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		if (IsValid(BB) && IsValid(InActor))
		{
			BB->SetValueAsVector(TEXT("LastKnownLocation"),
				Stimulus.StimulusLocation);
		}

		SetTargetActor(InActor);
	}
	else
	{
		if (InActor == MonsterComponent->GetTargetActor())
		{
			if (IsValid(BB) && IsValid(InActor))
			{
				BB->SetValueAsVector(TEXT("LastKnownLocation"),
					Stimulus.StimulusLocation);
			}
			ClearTargetActor();
		}
	}
}
