// Fill out your copyright notice in the Description page of Project Settings.


#include "BossMonster/GroundBossBase.h"
#include "Component/BossComponent.h"

AGroundBossBase::AGroundBossBase()
{
    BossComponent = CreateDefaultSubobject<UBossComponent>(TEXT("BossComponent"));
}

void AGroundBossBase::OnSpawnFromPool_Implementation()
{
	Super::OnSpawnFromPool_Implementation();

	if (IsValid(BossComponent))
	{
		BossComponent->RestartEncounterFromPool();
	}
}

void AGroundBossBase::OnReturnToPool_Implementation()
{
	if (IsValid(BossComponent))
	{
		BossComponent->PrepareForPoolReturn();
	}

	Super::OnReturnToPool_Implementation();
}
