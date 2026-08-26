// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterBase.h"

// Sets default values
AMonsterBase::AMonsterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMonsterBase::SetMonsterState(EMonsterState NewState)
{
}

void AMonsterBase::SetTarget(AActor* NewTarget)
{
}

void AMonsterBase::ClearTarget()
{
}

AActor* AMonsterBase::GetTargetActor()
{
	return nullptr;
}

float AMonsterBase::GetDistanceToTarget()
{
	return 0.0f;
}

// Called to bind functionality to input
void AMonsterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

