// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SubSystem/MonsterSpawnSubsystem.h"

#include "Monster/MonsterSpawnZone.h"

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
	for (AMonsterSpawnZone* Zone : RegisteredZones)
	{
		if (IsValid(Zone))
		{
			Zone->ResetZone();
		}
	}
}
