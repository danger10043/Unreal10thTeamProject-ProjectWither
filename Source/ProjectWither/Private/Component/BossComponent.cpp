// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BossComponent.h"

// Sets default values for this component's properties
UBossComponent::UBossComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UBossComponent::StartEncounter()
{
}

void UBossComponent::FinishPhaseTransition()
{
}


// Called when the game starts
void UBossComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UBossComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
}

void UBossComponent::HandleBossDeath()
{
}

void UBossComponent::SetPhase(EBossPhase NewPhase)
{
}

