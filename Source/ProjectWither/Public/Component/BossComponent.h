// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/BossPhaseEnums.h"
#include "TimerManager.h"
#include "BossComponent.generated.h"

class UAnimInstance;
class UAnimMontage;

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

    UPROPERTY(
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Boss",
        meta = (AllowPrivateAccess = "true")
    )
    EBossPhase CurrentPhase = EBossPhase::Phase1;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    float Phase2HealthRatio = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    TObjectPtr<UAnimMontage> PhaseTransitionMontage = nullptr;

    // Fallback for missing callbacks or looping montages. Extended for long montages.
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase", meta = (ClampMin = "0.1", Units = "s"))
    float PhaseTransitionTimeout = 10.0f;

    TWeakObjectPtr<UAnimInstance> TransitionAnimInstance;

    FTimerHandle PhaseTransitionTimerHandle;

    bool bPhase2Triggered = false;
};
