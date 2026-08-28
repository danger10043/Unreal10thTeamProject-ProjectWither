// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "MonsterBase.generated.h"

UCLASS()
class PROJECTWITHER_API AMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterBase();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetMonsterState(EMonsterState NewState); // 몬스터 상태 변경

	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* NewTarget); // 현재 타겟 설정

	UFUNCTION(BlueprintCallable)
	void ClearTarget(); // 현재 타겟 제거

	UFUNCTION(BlueprintCallable)
	AActor* GetTargetActor(); // 현재 타겟 반환

	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget(); // 현재 타겟까지 거리 반환

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	int32 MonsterId = 0; // 몬스터 고유 ID

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	EMonsterState MonsterState = EMonsterState::Idle; // 현재 몬스터 상태

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	//TObjectPtr<UStatComponent> StatComponent = nullptr; // 스탯 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	FVector SpawnLocation = FVector(0, 0, 0); // 최초 생성 위치

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	TObjectPtr<AActor> TargetActor = nullptr; // 현재 추적 또는 공격 대상

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	bool bIsDead = false; // 사망 여부

private:
	float InValidTargetActor = -1.0f;
};
