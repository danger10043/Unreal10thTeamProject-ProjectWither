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
	SnapToFloor(DeltaTime);
}



// Called to bind functionality to input
void AMonsterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMonsterPawn::SnapToFloor(float DeltaTime)
{
	FVector Origin = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	TArray<FVector> TraceOffsets = {
		FVector::ZeroVector,
		Forward * TraceOffsetRadius,
		-Forward * TraceOffsetRadius,
		Right * TraceOffsetRadius,
		-Right * TraceOffsetRadius
	};

	TArray<float> HitHeights;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	for (const FVector& Offset : TraceOffsets)
	{
		FVector Start = Origin + Offset;
		FVector End = Start - FVector(0.f, 0.f, FloorTraceDistance);

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			HitHeights.Add(Hit.Location.Z);
		}
	}

	if (HitHeights.Num() > 0)
	{
		float TargetZ = FMath::Max(HitHeights);
		FVector TargetLocation = FVector(Origin.X, Origin.Y, TargetZ + HeightAboveFloor);
		FVector NewLocation = FMath::VInterpTo(Origin, TargetLocation, DeltaTime, 10.f);
		SetActorLocation(NewLocation, true);
	}
}
