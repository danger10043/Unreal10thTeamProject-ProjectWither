// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnZone.generated.h"

class USceneComponent;

// 스폰 존 하나의 몬스터 배치 정보 (몬스터 클래스 + 마리 수)
USTRUCT(BlueprintType)
struct FMonsterSpawnSlot
{
	GENERATED_BODY()

	// 오브젝트 풀(Object Pool Settings)에 등록되어 있어야 하는 몬스터 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawn")
	TSubclassOf<AActor> MonsterClass;

	// 이 클래스를 몇 마리 스폰할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawn", meta = (ClampMin = "1"))
	int32 Count = 1;
};

/*
* 특정 구역에 몬스터를 배치하는 스폰 존 액터.
* 레벨 시작 시 SpawnSlots에 정의된 몬스터를 존 중심 기준 SpawnRadius 반경 안에
* 무작위 위치로 오브젝트 풀에서 스폰하고, UMonsterSpawnSubsystem의 리스폰 요청(ResetZone)을
* 받으면 지금까지 스폰한 몬스터를 전부 풀로 반환한 뒤 다시 무작위 배치한다.
*/
UCLASS()
class PROJECTWITHER_API AMonsterSpawnZone : public AActor
{
	GENERATED_BODY()

public:
	AMonsterSpawnZone();

	// 이 존이 스폰한 몬스터를 모두 풀로 반환한 뒤 SpawnSlots 그대로 다시 스폰
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void ResetZone();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SpawnAll();
	void ReturnAllToPool();

	// 겹치지 않는 스폰 위치를 존 중심 기준 반경 안에서 찾기
	FVector FindRandomSpawnLocation(const TArray<FVector>& UsedLocations) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Monster Spawn|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, Category = "Monster Spawn")
	TArray<FMonsterSpawnSlot> SpawnSlots;

	// 존 중심(액터 위치) 기준 몬스터를 흩뿌릴 반경
	UPROPERTY(EditAnywhere, Category = "Monster Spawn", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnRadius = 500.0f;

	// 같은 스폰에서 몬스터끼리 겹치지 않도록 유지할 최소 간격
	UPROPERTY(EditAnywhere, Category = "Monster Spawn", meta = (ClampMin = "0.0", Units = "cm"))
	float MinSpawnSpacing = 150.0f;

	// 이 존이 현재 스폰해서 관리 중인 몬스터
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedMonsters;
};
