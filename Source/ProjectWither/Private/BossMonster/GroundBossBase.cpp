// Fill out your copyright notice in the Description page of Project Settings.


#include "BossMonster/GroundBossBase.h"
#include "Component/BossComponent.h"
#include "Engine/World.h"

AGroundBossBase::AGroundBossBase()
{
    BossComponent = CreateDefaultSubobject<UBossComponent>(TEXT("BossComponent"));
}

void AGroundBossBase::OnSpawnFromPool_Implementation()
{
	// The pool makes actors visible before this callback. Keep the boss hidden
	// until the entrance montage has produced its first pose to prevent a
	// one-frame flash at the pool spawn transform.
	SetActorHiddenInGame(true);
	Super::OnSpawnFromPool_Implementation();

	if (IsValid(BossComponent))
	{
		BossComponent->RestartEncounterFromPool();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnRevealTimerHandle);
		SpawnRevealTimerHandle = World->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				SetActorHiddenInGame(false);
			}));
	}
	else
	{
		SetActorHiddenInGame(false);
	}
}

void AGroundBossBase::OnReturnToPool_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnRevealTimerHandle);
	}

	if (IsValid(BossComponent))
	{
		BossComponent->PrepareForPoolReturn();
	}

	Super::OnReturnToPool_Implementation();
}
