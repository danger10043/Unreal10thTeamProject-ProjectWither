// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SubSystem/MonsterSpawnSubsystem.h"

#include "Monster/MonsterSpawnZone.h"
#include "TimerManager.h"

void UMonsterSpawnSubsystem::RegisterZone(AMonsterSpawnZone* Zone)
{
	if (IsValid(Zone))
	{
		RegisteredZones.AddUnique(Zone);
	}
}

void UMonsterSpawnSubsystem::UnregisterZone(AMonsterSpawnZone* Zone)
{
	RegisteredZones.Remove(Zone);
}

void UMonsterSpawnSubsystem::RespawnAllZones()
{
	// 중복 요청은 이미 예약된 한 번의 리스폰으로 합친다.
	if (bRespawnPending)
	{
		return;
	}

	bRespawnPending = true;

	// 모든 BT를 먼저 완전히 중단하고 풀에 반환한다.
	for (AMonsterSpawnZone* Zone : RegisteredZones)
	{
		if (IsValid(Zone))
		{
			Zone->ReturnMonstersToPool();
		}
	}

	// 같은 프레임에 Stop/Restart가 겹치면 BT가 중단 상태에 남을 수 있으므로
	// 실제 재활성화는 다음 틱으로 넘긴다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			this, &UMonsterSpawnSubsystem::FinishRespawnAllZones);
	}
	else
	{
		FinishRespawnAllZones();
	}
}

void UMonsterSpawnSubsystem::FinishRespawnAllZones()
{
	bRespawnPending = false;

	for (AMonsterSpawnZone* Zone : RegisteredZones)
	{
		if (IsValid(Zone))
		{
			Zone->SpawnMonstersFromPool();
		}
	}
}
