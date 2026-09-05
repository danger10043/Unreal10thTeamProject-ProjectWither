// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BossComponent.h"

#include "Monster/MonsterCharacterBase.h"
#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

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
		const UStatComponent* Stat = GetOwner()->FindComponentByClass<UStatComponent>();
		if (IsValid(Stat) && Stat->IsHealthZero()) return;
		SetPhase(EBossPhase::Phase2);
	}
}

void UBossComponent::HandlePhaseTransitionTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("Boss phase transition timed out: %s"), *GetNameSafe(GetOwner()));
	FinishPhaseTransition();
}

void UBossComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPhaseTransitionMontage();
	if (IsTransitioning() && IsValid(GetOwner()))
	{
		if (UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>())
		{
			Monster->SetCombatLocked(false);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseTransitionTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
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
	if (bPhase2Triggered || CurrentPhase != EBossPhase::Phase1 || MaxHealth <= 0.0f)
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
	if (PreviousPhase == EBossPhase::Transition)
	{
		StopPhaseTransitionMontage();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseTransitionTimerHandle);
		if (IsTransitioning())
		{
			World->GetTimerManager().SetTimer(PhaseTransitionTimerHandle, this,
				&UBossComponent::HandlePhaseTransitionTimeout,
				FMath::Max(0.1f, PhaseTransitionTimeout), false);
		}
	}

	// The boss owns the phase rule; the monster only knows about a combat lock.
	// Lock before cancellation callbacks or BP can try to start another attack.
	if (UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>())
	{
		Monster->SetCombatLocked(IsTransitioning());
		if (IsTransitioning())
		{
			Monster->CancelAttack();
			Monster->FinishAttack();
			Monster->CancelSearch();
		}
	}

	// Cancellation callbacks can synchronously change the phase (for example death).
	if (CurrentPhase != NewPhase) return;

	OnBossPhaseChanged.Broadcast(PreviousPhase, CurrentPhase);
	if (NewPhase == EBossPhase::Transition && IsTransitioning())
	{
		PlayPhaseTransitionMontage();
	}
}

void UBossComponent::PlayPhaseTransitionMontage()
{
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (!IsValid(PhaseTransitionMontage) || !IsValid(AnimInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss transition montage or AnimInstance missing: %s"), *GetNameSafe(GetOwner()));
		FinishPhaseTransition();
		return;
	}

	const float Duration = AnimInstance->Montage_Play(
		PhaseTransitionMontage, 1.0f, EMontagePlayReturnType::Duration);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss transition montage failed to play: %s"), *GetNameSafe(GetOwner()));
		FinishPhaseTransition();
		return;
	}

	// Montage callbacks can change the phase while Montage_Play stops old montages.
	if (!IsTransitioning())
	{
		AnimInstance->Montage_Stop(0.0f, PhaseTransitionMontage);
		return;
	}

	TransitionAnimInstance = AnimInstance;
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBossComponent::HandleTransitionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, PhaseTransitionMontage);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PhaseTransitionTimerHandle, this,
			&UBossComponent::HandlePhaseTransitionTimeout,
			FMath::Max(PhaseTransitionTimeout, Duration + 1.0f), false);
	}
}

void UBossComponent::HandleTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PhaseTransitionMontage) return;
	TransitionAnimInstance.Reset();
	// Both natural completion and interruption release the transition lock.
	FinishPhaseTransition();
}

void UBossComponent::StopPhaseTransitionMontage()
{
	UAnimInstance* AnimInstance = TransitionAnimInstance.Get();
	TransitionAnimInstance.Reset();
	if (IsValid(AnimInstance) && IsValid(PhaseTransitionMontage))
	{
		// Unbind first: death, timeout, or a manual finish must not reenter SetPhase.
		FOnMontageEnded EmptyDelegate;
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, PhaseTransitionMontage);
		AnimInstance->Montage_Stop(0.0f, PhaseTransitionMontage);
	}
}

