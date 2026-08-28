// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MonsterPawn.generated.h"

UCLASS()
class PROJECTWITHER_API AMonsterPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMonsterPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RotationInterpSpeed = 5.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float FloorTraceDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float HeightAboveFloor = 0.f; // 캡슐 피벗이 바닥 기준 몇 cm 위인지에 맞춰 조정

	void SnapToFloor(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TraceOffsetRadius = 800.f; // 몬스터 몸통 크기에 맞춰 조정
};
