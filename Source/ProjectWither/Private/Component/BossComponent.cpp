// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BossComponent.h"

#include "Monster/MonsterCharacterBase.h"
#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"

// Sets default values for this component's properties
UBossComponent::UBossComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UBossComponent::StartEncounter()
{
	SetPhase(EBossPhase::Phase1);
	OnBossEncounterStarted.Broadcast();
}

void UBossComponent::FinishPhaseTransition()
{
	if (CurrentPhase == EBossPhase::Transition)
	{
		SetPhase(EBossPhase::Phase2);
	}
}

void UBossComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
    {
        return;
    }

	if (OwnerActor->GetClass()->ImplementsInterface(UStatComponentUserInterface::StaticClass()))
    {
		if (UStatComponent* Stat = IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor))
        {
            Stat->OnHealthChanged.AddUniqueDynamic(
                this,
                &UBossComponent::HandleHealthChanged
            );
        }
    }

	if (AMonsterCharacterBase* Monster = Cast<AMonsterCharacterBase>(OwnerActor))
    {
		if (UMonsterComponent* MonsterComp = Monster->GetMonsterComponent())
        {
            MonsterComp->OnMonsterDied.AddUniqueDynamic(
                this,
                &UBossComponent::HandleBossDeath
            );
        }
    }
}

void UBossComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	if (bPhase2Triggered || MaxHealth <= 0.0f)
	{
		return;
	}

	const float HealthRatio = CurrentHealth / MaxHealth;

	if (HealthRatio <= Phase2HealthRatio && CurrentHealth > 0.0f)
	{
		bPhase2Triggered = true;
		SetPhase(EBossPhase::Transition);
	}
}

void UBossComponent::HandleBossDeath()
{
	SetPhase(EBossPhase::Dead);
	OnBossEncounterEnded.Broadcast();
}

void UBossComponent::SetPhase(EBossPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	const EBossPhase PreviousPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	OnBossPhaseChanged.Broadcast(PreviousPhase, CurrentPhase);
}

