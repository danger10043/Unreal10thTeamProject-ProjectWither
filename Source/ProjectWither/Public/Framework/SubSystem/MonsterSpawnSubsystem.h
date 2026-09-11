// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MonsterSpawnSubsystem.generated.h"

class AMonsterSpawnZone;

// 레벨에 배치된 모든 몬스터 스폰 존을 등록/관리하고, 전체 리스폰 요청을 전달하는 Subsystem
UCLASS()
class PROJECTWITHER_API UMonsterSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 스폰 존이 자신의 BeginPlay에서 호출해 스스로 등록
	void RegisterZone(AMonsterSpawnZone* Zone);

	// 스폰 존이 자신의 EndPlay에서 호출해 등록 해제
	void UnregisterZone(AMonsterSpawnZone* Zone);

	// 등록된 모든 스폰 존의 몬스터를 전부 풀로 반환한 뒤 다시 스폰 (세이브 포인트 휴식 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Monster Spawn")
	void RespawnAllZones();

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AMonsterSpawnZone>> RegisteredZones;
};
