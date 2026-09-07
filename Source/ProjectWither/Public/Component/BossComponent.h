// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/BossPhaseEnums.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BossComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UBrainComponent;
class UBossDataAsset;
struct FBossPhaseSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnBossPhaseChanged,
    EBossPhase, PreviousPhase,
    EBossPhase, NewPhase
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossEncounterStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossEncounterEnded);

UCLASS(ClassGroup = (Boss), meta = (BlueprintSpawnableComponent))
class PROJECTWITHER_API UBossComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBossComponent();

    UFUNCTION(BlueprintPure, Category = "Boss")
    EBossPhase GetCurrentPhase() const
    {
        return CurrentPhase;
    }

    UFUNCTION(BlueprintPure, Category = "Boss")
    bool IsTransitioning() const
    {
        return CurrentPhase == EBossPhase::Transition;
    }

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void FinishPhaseTransition();

    UPROPERTY(BlueprintAssignable, Category = "Boss|Event")
    FOnBossPhaseChanged OnBossPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Boss|Event")
    FOnBossEncounterStarted OnBossEncounterStarted;

    UPROPERTY(BlueprintAssignable, Category = "Boss|Event")
    FOnBossEncounterEnded OnBossEncounterEnded;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UFUNCTION()
    void HandleHealthChanged(
        float CurrentHealth,
        float MaxHealth,
        float ChangedAmount
    );

    UFUNCTION()
    void HandleBossDeath();

    void SetPhase(EBossPhase NewPhase);
    void HandlePhaseTransitionTimeout();
    void PlayPhaseTransitionMontage();
    void StopPhaseTransitionMontage();
    void HandleTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void LockTransitionMovement();
    void ReleaseTransitionMovement(bool bRestore);
    UBossDataAsset* GetBossData() const;
    UAnimMontage* GetPhaseTransitionMontage() const;
    const FBossPhaseSettings* GetPhaseSettings(EBossPhase Phase) const;
    void ApplyPhaseSettings(EBossPhase Phase);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = "true"))
    EBossPhase CurrentPhase = EBossPhase::Phase1;

    // Fallback for missing callbacks or looping montages. Extended for long montages.
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase", meta = (ClampMin = "0.1", Units = "s"))
    float PhaseTransitionTimeout = 10.0f;

    TWeakObjectPtr<UAnimInstance> TransitionAnimInstance;
    TWeakObjectPtr<UCharacterMovementComponent> TransitionMovement;
    TWeakObjectPtr<UBrainComponent> TransitionBrain;
    TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
    uint8 PreviousCustomMovementMode = 0;

    FTimerHandle PhaseTransitionTimerHandle;

    bool bPhase2Triggered = false;
};
