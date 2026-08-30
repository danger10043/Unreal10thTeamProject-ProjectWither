// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "MonsterComponent.generated.h"

class UStatComponent;
class UItemDataAsset;
class UDataTable;
class APickupItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterDied);

UCLASS(ClassGroup=(Monster), meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UMonsterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Shared gameplay state for both Character and Pawn monsters.
	UMonsterComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Both monster actor types forward their damage here.
    float ApplyMonsterDamage(float Damage);

    UFUNCTION(BlueprintPure, Category = "Monster")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    int32 GetMonsterId() const { return MonsterId; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    EMonsterState GetMonsterState() const { return MonsterState; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    FVector GetSpawnLocation() const { return SpawnLocation; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    float GetAllowRange() const { return AllowRange; }

    UPROPERTY(BlueprintAssignable, Category = "Monster")
    FOnMonsterDied OnMonsterDied;

	UFUNCTION(BlueprintCallable)
	void SetMonsterState(EMonsterState NewState); // 몬스터 상태 변경

	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* NewTarget); // 현재 타겟 설정

	UFUNCTION(BlueprintCallable)
	void ClearTarget(); // 현재 타겟 제거

	UFUNCTION(BlueprintCallable)
	AActor* GetTargetActor() const; // 현재 타겟 반환

	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget() const; // 현재 타겟까지 거리 반환

	UFUNCTION(BlueprintCallable)
	void CalculateDrops();		// 드랍 여부 및 개수 결정

	UFUNCTION(BlueprintCallable)
	void DropItems();			// 계산된 아이템을 월드에 생성

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	int32 MonsterId = 0; // 몬스터 고유 ID

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	EMonsterState MonsterState = EMonsterState::Idle; // 현재 몬스터 상태

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	TObjectPtr<UStatComponent> StatComponent = nullptr; // 스탯 컴포넌트

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	FVector SpawnLocation = FVector(0, 0, 0); // 최초 생성 위치

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	TObjectPtr<AActor> TargetActor = nullptr; // 현재 추적 또는 공격 대상

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	bool bIsDead = false; // 사망 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float AllowRange = 100.0f;	// 플레이어에게 최대한 접근할 수 있는 거리

	// Drop --------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<UDataTable> ItemDropTable = nullptr;	// 몬스터 드랍 테이블

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<APickupItem> ItemPickupClass;	// 아이템 픽업 클래스

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropRange = 100.0f;		// 드랍 아이템 랜덤 범위 (X, Y)

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropHeight = 20.0f;		// 드랍 아이템 랜덤 높이 (Z)
	// -------------------------------------------------------------------------

private:
	UFUNCTION()
	void HandleDeath();

	UPROPERTY(Transient)
	TMap<TObjectPtr<UItemDataAsset>, int32> DropItem;	// 계산 후 확정된 드랍 아이템들
};
