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
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Character.h"
#include "DataAsset/BossDataAsset.h"

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
	ReleaseTransitionMovement(false);
	if (IsTransitioning() && IsValid(GetOwner()))
	{
		if (UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>())
		{
			Monster->SetCombatLocked(false);
			Monster->SetDamageLocked(false);
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

	if (!IsValid(GetBossData()))
	{
		UE_LOG(LogTemp, Error, TEXT("BossData is missing: %s"), *GetNameSafe(GetOwner()));
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

	ApplyPhaseSettings(EBossPhase::Phase1);
}

void UBossComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	if (bPhase2Triggered || CurrentPhase != EBossPhase::Phase1 || MaxHealth <= 0.0f)
	{
		return;
	}

	const float HealthRatio = CurrentHealth / MaxHealth;

	const UBossDataAsset* BossData = GetBossData();
	if (!IsValid(BossData))
	{
		return;
	}

	if (HealthRatio <= BossData->Phase2HealthRatio &&
		CurrentHealth > 0.0f)
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
	if (NewPhase == EBossPhase::Phase1 || NewPhase == EBossPhase::Phase2)
	{
		ApplyPhaseSettings(NewPhase);
	}
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
		Monster->SetDamageLocked(IsTransitioning());
		if (IsTransitioning())
		{
			Monster->CancelAttack();
			Monster->FinishAttack();
			Monster->CancelSearch();
		}
	}

	// Cancellation callbacks can synchronously change the phase (for example death).
	if (CurrentPhase != NewPhase) return;

	if (IsTransitioning())
	{
		LockTransitionMovement();
	}
	else if (PreviousPhase == EBossPhase::Transition)
	{
		ReleaseTransitionMovement(NewPhase != EBossPhase::Dead);
	}
	if (CurrentPhase != NewPhase) return;

	OnBossPhaseChanged.Broadcast(PreviousPhase, CurrentPhase);
	if (NewPhase == EBossPhase::Transition && IsTransitioning())
	{
		PlayPhaseTransitionMontage();
	}
}

void UBossComponent::PlayPhaseTransitionMontage()
{
	UAnimMontage* TransitionMontage = GetPhaseTransitionMontage();
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (!IsValid(TransitionMontage) || !IsValid(AnimInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss transition montage or AnimInstance missing: %s"), *GetNameSafe(GetOwner()));
		FinishPhaseTransition();
		return;
	}

	const float Duration = AnimInstance->Montage_Play(
		TransitionMontage, 1.0f, EMontagePlayReturnType::Duration);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss transition montage failed to play: %s"), *GetNameSafe(GetOwner()));
		FinishPhaseTransition();
		return;
	}

	// Montage callbacks can change the phase while Montage_Play stops old montages.
	if (!IsTransitioning())
	{
		AnimInstance->Montage_Stop(0.0f, TransitionMontage);
		return;
	}

	TransitionAnimInstance = AnimInstance;
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBossComponent::HandleTransitionMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, TransitionMontage);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PhaseTransitionTimerHandle, this,
			&UBossComponent::HandlePhaseTransitionTimeout,
			FMath::Max(PhaseTransitionTimeout, Duration + 1.0f), false);
	}
}

void UBossComponent::HandleTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != GetPhaseTransitionMontage()) return;
	TransitionAnimInstance.Reset();
	// Both natural completion and interruption release the transition lock.
	FinishPhaseTransition();
}

void UBossComponent::StopPhaseTransitionMontage()
{
	UAnimMontage* TransitionMontage = GetPhaseTransitionMontage();
	UAnimInstance* AnimInstance = TransitionAnimInstance.Get();
	TransitionAnimInstance.Reset();
	if (IsValid(AnimInstance) && IsValid(TransitionMontage))
	{
		// Unbind first: death, timeout, or a manual finish must not reenter SetPhase.
		FOnMontageEnded EmptyDelegate;
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, TransitionMontage);
		AnimInstance->Montage_Stop(0.0f, TransitionMontage);
	}
}

UAnimMontage* UBossComponent::GetPhaseTransitionMontage() const
{
	const UBossDataAsset* BossData = GetBossData();
	return IsValid(BossData) ? BossData->PhaseTransitionMontage.Get() : nullptr;
}

UBossDataAsset* UBossComponent::GetBossData() const
{
	if (!IsValid(GetOwner())) return nullptr;

	const UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>();
	return IsValid(Monster) ? Cast<UBossDataAsset>(Monster->GetMonsterData()) : nullptr;
}

const FBossPhaseSettings* UBossComponent::GetPhaseSettings(EBossPhase Phase) const
{
	const UBossDataAsset* BossData = GetBossData();
	if (!IsValid(BossData))
	{
		return nullptr;
	}

	switch (Phase)
	{
	case EBossPhase::Phase1:
		return &BossData->BaseSettings;
	case EBossPhase::Phase2:
		return &BossData->Phase2Settings;
	default:
		return nullptr;
	}
}

void UBossComponent::ApplyPhaseSettings(EBossPhase Phase)
{
	const FBossPhaseSettings* Settings = GetPhaseSettings(Phase);
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!Settings || !IsValid(OwnerCharacter))
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = FMath::Max(0.0f, Settings->MoveSpeed);
	}

	if (UMonsterComponent* Monster = OwnerCharacter->FindComponentByClass<UMonsterComponent>())
	{
		Monster->SetAttackCooldown(Settings->AttackCooldown);
		Monster->SetAttackRange(Settings->AttackRange);
		Monster->SetAllowRange(Settings->AllowRange);
	}

	if (UStatComponent* Stat = OwnerCharacter->FindComponentByClass<UStatComponent>())
	{
		Stat->SetCombatMultipliers(
			Settings->AttackPowerMultiplier,
			Settings->DefenseMultiplier);
	}
}

void UBossComponent::LockTransitionMovement()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn)) return;

	// Disable Character movement, including root motion, independently of the
	// common montage callbacks that may reactivate the movement component.
	if (UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
	{
		if (!TransitionMovement.IsValid())
		{
			TransitionMovement = Movement;
			PreviousMovementMode = Movement->MovementMode;
			PreviousCustomMovementMode = Movement->CustomMovementMode;
		}
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	Pawn->ConsumeMovementInputVector();

	if (AAIController* AI = Cast<AAIController>(Pawn->GetController()))
	{
		UBrainComponent* Brain = AI->GetBrainComponent();
		if (IsValid(Brain) && Brain->IsRunning() && !Brain->IsPaused())
		{
			TransitionBrain = Brain;
			Brain->PauseLogic(TEXT("Boss phase transition"));
		}
		AI->StopMovement();
	}
}

void UBossComponent::ReleaseTransitionMovement(bool bRestore)
{
	const UMonsterComponent* Monster = IsValid(GetOwner())
		? GetOwner()->FindComponentByClass<UMonsterComponent>() : nullptr;
	const UStatComponent* Stat = IsValid(GetOwner())
		? GetOwner()->FindComponentByClass<UStatComponent>() : nullptr;
	const bool bCanRestore = bRestore && IsValid(GetOwner())
		&& (!IsValid(Monster) || !Monster->IsDead())
		&& (!IsValid(Stat) || !Stat->IsHealthZero());

	UCharacterMovementComponent* Movement = TransitionMovement.Get();
	UBrainComponent* Brain = TransitionBrain.Get();
	TransitionMovement.Reset();
	TransitionBrain.Reset();
	if (!bCanRestore) return;

	if (IsValid(Movement))
	{
		Movement->SetMovementMode(PreviousMovementMode, PreviousCustomMovementMode);
	}
	// Resume only logic that this component paused. Never restart stopped AI.
	if (IsValid(Brain) && Brain->IsPaused())
	{
		Brain->ResumeLogic(TEXT("Boss phase transition finished"));
	}
}

