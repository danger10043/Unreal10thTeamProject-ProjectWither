// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_MonsterTurnToTarget.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"

UBTTask_MonsterTurnToTarget::UBTTask_MonsterTurnToTarget()
{
	NodeName = TEXT("Monster Turn To Target");

	bCreateNodeInstance = true;
	bNotifyTick = true;

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(
			UBTTask_MonsterTurnToTarget,
			TargetActorKey),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type UBTTask_MonsterTurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = IsValid(AIController) ?
		AIController->GetPawn() : nullptr;

	UBlackboardComponent* Blackboard =
		OwnerComp.GetBlackboardComponent();

	AActor* TargetActor = IsValid(Blackboard)
		? Cast<AActor>(
			Blackboard->GetValueAsObject(
				TargetActorKey.SelectedKeyName))
		: nullptr;

	if (!IsValid(Pawn) || !IsValid(TargetActor) || !IsValid(TurnMontage))
	{
		return EBTNodeResult::Failed;
	}

	const FVector Direction =
		TargetActor->GetActorLocation() -
		Pawn->GetActorLocation();

	if (Direction.IsNearlyZero())
	{
		return EBTNodeResult::Succeeded;
	}

	StartRotation = Pawn->GetActorRotation();
	TargetRotation = Direction.Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	DeltaYaw = FMath::FindDeltaAngleDegrees(
		StartRotation.Yaw,
		TargetRotation.Yaw);

	if (FMath::Abs(DeltaYaw) < MinimumTurnAngle)
	{
		Pawn->SetActorRotation(TargetRotation);
		return EBTNodeResult::Succeeded;
	}

	FName SectionName;

	if (FMath::Abs(DeltaYaw) < 135.0f)
	{
		SectionName = DeltaYaw > 0.0f
			? TEXT("Turn_Right_90")
			: TEXT("Turn_Left_90");
	}
	else
	{
		SectionName = DeltaYaw > 0.0f
			? TEXT("Turn_Right_180")
			: TEXT("Turn_Left_180");
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_MonsterTurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!IsValid(CachedPawn) || TurnDuration <= 0.0f)
	{
		FinishTurn(false);
		return;
	}

	ElapsedTime += DeltaSeconds;

	const float Alpha = FMath::Clamp(
		ElapsedTime / TurnDuration,
		0.0f,
		1.0f);

	const float SmoothAlpha =
		FMath::InterpEaseInOut(
			0.0f,
			1.0f,
			Alpha,
			2.0f);

	const float NewYaw =
		StartRotation.Yaw +
		DeltaYaw * SmoothAlpha;

	CachedPawn->SetActorRotation(
		FRotator(0.0f, NewYaw, 0.0f));
}

EBTNodeResult::Type UBTTask_MonsterTurnToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Type();
}

void UBTTask_MonsterTurnToTarget::FinishTurn(bool bSucceeded)
{
}

void UBTTask_MonsterTurnToTarget::RestoreMovement()
{
}

void UBTTask_MonsterTurnToTarget::OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
}
