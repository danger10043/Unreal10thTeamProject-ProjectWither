// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterSpawnZone.h"

#include "Components/SceneComponent.h"
#include "Component/MonsterComponent.h"
#include "Framework/SubSystem/MonsterSpawnSubsystem.h"
#include "Framework/SubSystem/ObjectPoolSubsystem.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"

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

void AMonsterSpawnZone::ReturnMonstersToPool()
{
	ReturnAllToPool();
}

void AMonsterSpawnZone::SpawnMonstersFromPool()
{
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
			// Character 기반 지상 몬스터는 반드시 NavMesh 위에 배치한다.
			// Pawn 기반 비행 몬스터는 기존 3D 이동 기준 위치를 유지한다.
			const bool bGroundMonster =
				Slot.MonsterClass->IsChildOf(ACharacter::StaticClass());
			const FVector SpawnLocation = FindRandomSpawnLocation(
				UsedLocations, bGroundMonster);
			const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);
			const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

			if (AActor* Spawned = PoolSubsystem->Spawn(Slot.MonsterClass, SpawnTransform))
			{
				if (UMonsterComponent* MonsterComponent = Spawned->FindComponentByClass<UMonsterComponent>())
				{
					MonsterComponent->ApplySpawnZoneScaling(
						HealthMultiplier, AttackMultiplier, DefenseMultiplier, GoldMultiplier);
				}
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

FVector AMonsterSpawnZone::FindRandomSpawnLocation(
	const TArray<FVector>& UsedLocations,
	bool bProjectToNavigation) const
{
	const FVector Origin = GetActorLocation();
	UNavigationSystemV1* NavigationSystem = bProjectToNavigation
		? FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld())
		: nullptr;

	constexpr int32 MaxAttempts = 10;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const FVector2D RandomOffset = FMath::RandPointInCircle(SpawnRadius);
		FVector Candidate = Origin + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);

		if (bProjectToNavigation)
		{
			FNavLocation ProjectedLocation;
			if (!IsValid(NavigationSystem) ||
				!NavigationSystem->ProjectPointToNavigation(
					Candidate, ProjectedLocation, FVector(150.0f, 150.0f, 500.0f)))
			{
				continue;
			}
			Candidate = ProjectedLocation.Location;
		}

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

	// 반경 안에서 겹치지 않는 자리를 못 찾으면 NavMesh상의 존 중심을 우선 사용한다.
	if (bProjectToNavigation && IsValid(NavigationSystem))
	{
		FNavLocation ProjectedOrigin;
		if (NavigationSystem->ProjectPointToNavigation(
			Origin, ProjectedOrigin, FVector(500.0f, 500.0f, 1000.0f)))
		{
			return ProjectedOrigin.Location;
		}
	}

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
