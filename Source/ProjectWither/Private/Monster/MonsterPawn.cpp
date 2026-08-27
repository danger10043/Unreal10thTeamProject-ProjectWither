// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterPawn.h"

// Sets default values
AMonsterPawn::AMonsterPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void AMonsterPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMonsterPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Velocity = GetVelocity();
	if (!Velocity.IsNearlyZero())
	{
		FRotator TargetRotation = Velocity.Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed);
		SetActorRotation(NewRotation);
	}
}

// Called to bind functionality to input
void AMonsterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

