// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterSpawnZone.h"

#include "Components/SceneComponent.h"
#include "Component/MonsterComponent.h"
#include "Framework/SubSystem/MonsterSpawnSubsystem.h"
#include "Framework/SubSystem/ObjectPoolSubsystem.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Engine/OverlapResult.h"

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
			FVector SpawnLocation;
			if (!FindRandomSpawnLocation(
				UsedLocations, bGroundMonster, SpawnLocation))
			{
				UE_LOG(LogTemp, Warning,
					TEXT("AMonsterSpawnZone::SpawnAll - %s: 겹치지 않는 유효한 스폰 위치를 찾지 못해 %s 스폰을 건너뜁니다."),
					*GetName(), *Slot.MonsterClass->GetName());
				continue;
			}
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

bool AMonsterSpawnZone::FindRandomSpawnLocation(
	const TArray<FVector>& UsedLocations,
	bool bProjectToNavigation,
	FVector& OutSpawnLocation) const
{
	const FVector Origin = GetActorLocation();
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavigationSystem = bProjectToNavigation
		? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World)
		: nullptr;

	constexpr int32 MaxAttempts = 30;
	bool bHasFallbackCandidate = false;
	FVector BestFallbackCandidate = Origin;
	float BestFallbackClearanceSq = -1.0f;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		FVector Candidate;
		const float SearchRadiusScale = Attempt < 10
			? 1.0f
			: (Attempt < 20 ? 1.5f : 2.0f);
		const float EffectiveSpawnRadius = SpawnRadius * SearchRadiusScale;

		if (bProjectToNavigation)
		{
			FNavLocation ProjectedOrigin;
			FNavLocation ReachableLocation;
			if (!IsValid(NavigationSystem) ||
				!NavigationSystem->ProjectPointToNavigation(
					Origin, ProjectedOrigin, FVector(500.0f, 500.0f, 1000.0f)) ||
				!NavigationSystem->GetRandomReachablePointInRadius(
					ProjectedOrigin.Location, EffectiveSpawnRadius, ReachableLocation))
			{
				continue;
			}
			Candidate = ReachableLocation.Location;
		}
		else
		{
			const FVector2D RandomOffset = FMath::RandPointInCircle(EffectiveSpawnRadius);
			Candidate = Origin + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
		}

		const bool bOverlapsThisBatch = UsedLocations.ContainsByPredicate(
			[&Candidate, this](const FVector& Used)
			{
				return FVector::DistSquared(Candidate, Used) < FMath::Square(MinSpawnSpacing);
			});

		float CandidateClearanceSq = TNumericLimits<float>::Max();
		for (const FVector& Used : UsedLocations)
		{
			CandidateClearanceSq = FMath::Min(
				CandidateClearanceSq,
				FVector::DistSquared(Candidate, Used));
		}

		// 다른 스폰 존에서 먼저 생성된 몬스터까지 포함해 실제 Pawn과의 간격을 확인한다.
		bool bOverlapsExistingMonster = false;
		if (IsValid(World) && MinSpawnSpacing > 0.0f)
		{
			TArray<FOverlapResult> Overlaps;
			FCollisionObjectQueryParams ObjectQuery;
			ObjectQuery.AddObjectTypesToQuery(ECC_Pawn);
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MonsterSpawnSpacing), false, this);
			World->OverlapMultiByObjectType(
				Overlaps,
				Candidate,
				FQuat::Identity,
				ObjectQuery,
				FCollisionShape::MakeSphere(MinSpawnSpacing),
				QueryParams);

			// 플레이어나 다른 Pawn은 몬스터 배치 간격의 대상이 아니다.
			// 실제 활성 몬스터와 겹칠 때만 다음 후보를 찾는다.
			bOverlapsExistingMonster = Overlaps.ContainsByPredicate(
				[](const FOverlapResult& Result)
				{
					const AActor* OverlappedActor = Result.GetActor();
					return IsValid(OverlappedActor) &&
						IsValid(OverlappedActor->FindComponentByClass<UMonsterComponent>());
				});

			for (const FOverlapResult& Result : Overlaps)
			{
				const AActor* OverlappedActor = Result.GetActor();
				if (IsValid(OverlappedActor) &&
					IsValid(OverlappedActor->FindComponentByClass<UMonsterComponent>()))
				{
					CandidateClearanceSq = FMath::Min(
						CandidateClearanceSq,
						FVector::DistSquared(Candidate, OverlappedActor->GetActorLocation()));
				}
			}
		}

		if (!bOverlapsThisBatch && !bOverlapsExistingMonster)
		{
			OutSpawnLocation = Candidate;
			return true;
		}

		// 모든 후보가 겹칠 경우에도 가장 여유가 큰 위치를 보존한다.
		if (!bHasFallbackCandidate || CandidateClearanceSq > BestFallbackClearanceSq)
		{
			bHasFallbackCandidate = true;
			BestFallbackCandidate = Candidate;
			BestFallbackClearanceSq = CandidateClearanceSq;
		}
	}

	if (bHasFallbackCandidate)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%s: 최소 스폰 간격을 확보하지 못해 가장 여유가 큰 NavMesh 후보에 확정 스폰합니다."),
			*GetName());
		OutSpawnLocation = BestFallbackCandidate;
		return true;
	}

	// NavMesh 쿼리 자체가 실패해도 몬스터 수가 누락되지는 않게 한다.
	// 이 경고가 발생하면 해당 존 주변의 NavMesh 배치를 확인해야 한다.
	UE_LOG(LogTemp, Warning,
		TEXT("%s: 유효한 NavMesh 후보가 없어 스폰 존 원점에 확정 스폰합니다."),
		*GetName());
	OutSpawnLocation = Origin;
	return true;
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
