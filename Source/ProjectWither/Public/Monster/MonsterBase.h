// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "MonsterBase.generated.h"

class UStatComponent;
class UItemDataAsset;
class UDataTable;
class APickupItem;

UCLASS()
class PROJECTWITHER_API AMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	FVector SpawnLocation = FVector(0, 0, 0); // 최초 생성 위치

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	TObjectPtr<AActor> TargetActor = nullptr; // 현재 추적 또는 공격 대상

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	bool bIsDead = false; // 사망 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<UDataTable> ItemDropTable = nullptr;	// 몬스터 드랍 테이블

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<APickupItem> ItemPickupClass;	// 아이템 픽업 클래스

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropRange = 100.0f;		// 드랍 아이템 랜덤 범위 (X, Y)

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropHeight = 20.0f;		// 드랍 아이템 랜덤 높이 (Z)

private:
	float InValidTargetActor = -1.0f; // 타겟이 유효하지 않을때 반환용(private)

	UPROPERTY(Transient)
	TMap<TObjectPtr<UItemDataAsset>, int32> DropItem;	// 계산 후 확정된 드랍 아이템들
};
