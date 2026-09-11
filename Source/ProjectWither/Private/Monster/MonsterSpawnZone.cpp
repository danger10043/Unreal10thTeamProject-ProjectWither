// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterSpawnZone.h"

#include "Components/SceneComponent.h"
#include "Framework/SubSystem/MonsterSpawnSubsystem.h"
#include "Framework/SubSystem/ObjectPoolSubsystem.h"

AMonsterSpawnZone::AMonsterSpawnZone()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AMonsterSpawnZone::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UMonsterSpawnSubsystem* Subsystem = World->GetSubsystem<UMonsterSpawnSubsystem>())
		{
			Subsystem->RegisterZone(this);
		}
	}

	SpawnAll();
}

void AMonsterSpawnZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UMonsterSpawnSubsystem* Subsystem = World->GetSubsystem<UMonsterSpawnSubsystem>())
		{
			Subsystem->UnregisterZone(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMonsterSpawnZone::ResetZone()
{
	ReturnAllToPool();
	SpawnAll();
}

void AMonsterSpawnZone::SpawnAll()
{
	UWorld* World = GetWorld();

	UObjectPoolSubsystem* PoolSubsystem =
		IsValid(World) ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;

	if (!IsValid(PoolSubsystem))
	{
		return;
	}

	TArray<FVector> UsedLocations;

	for (const FMonsterSpawnSlot& Slot : SpawnSlots)
	{
		if (!Slot.MonsterClass)
		{
			continue;
		}

		for (int32 i = 0; i < Slot.Count; ++i)
		{
			const FVector SpawnLocation = FindRandomSpawnLocation(UsedLocations);
			const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
			const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

			if (AActor* Spawned = PoolSubsystem->Spawn(Slot.MonsterClass, SpawnTransform))
			{
				SpawnedMonsters.Add(Spawned);
				UsedLocations.Add(SpawnLocation);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("AMonsterSpawnZone::SpawnAll - %s: %s 스폰 실패 (Object Pool Settings에 등록되었는지 확인)"),
					*GetName(), *Slot.MonsterClass->GetName());
			}
		}
	}
}

FVector AMonsterSpawnZone::FindRandomSpawnLocation(const TArray<FVector>& UsedLocations) const
{
	const FVector Origin = GetActorLocation();

	constexpr int32 MaxAttempts = 10;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const FVector2D RandomOffset = FMath::RandPointInCircle(SpawnRadius);
		const FVector Candidate = Origin + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);

		const bool bOverlaps = UsedLocations.ContainsByPredicate(
			[&Candidate, this](const FVector& Used)
			{
				return FVector::DistSquared(Candidate, Used) < FMath::Square(MinSpawnSpacing);
			});

		if (!bOverlaps)
		{
			return Candidate;
		}
	}

	// 반경 안에서 겹치지 않는 자리를 못 찾으면 마지막 후보라도 사용
	const FVector2D FallbackOffset = FMath::RandPointInCircle(SpawnRadius);
	return Origin + FVector(FallbackOffset.X, FallbackOffset.Y, 0.0f);
}

void AMonsterSpawnZone::ReturnAllToPool()
{
	UWorld* World = GetWorld();

	UObjectPoolSubsystem* PoolSubsystem =
		IsValid(World) ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;

	if (IsValid(PoolSubsystem))
	{
		for (AActor* Monster : SpawnedMonsters)
		{
			if (IsValid(Monster))
			{
				PoolSubsystem->ReturnPool(Monster);
			}
		}
	}

	SpawnedMonsters.Reset();
}
