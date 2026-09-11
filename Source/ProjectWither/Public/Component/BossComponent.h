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
class UStatComponent;
class UMaterialInstanceDynamic;
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

    UFUNCTION(BlueprintPure, Category = "Boss")
    bool IsPlayingEntrance() const { return bEntrancePlaying; }

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void StartEncounter();

    // Resets a pooled boss and starts its entrance sequence again.
    void RestartEncounterFromPool();

    // Clears encounter-only state before the boss returns to the pool.
    void PrepareForPoolReturn();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    void FinishPhaseTransition();

	/** Shows the screen-fixed health bar and restarts its inactivity timer. */
	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	void ShowHealthBar();

	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	void HideHealthBar();

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
	void CheckHealthBarVisibility();
	void RefreshHealthBar(float CurrentHealth, float MaxHealth);

    void SetPhase(EBossPhase NewPhase);
    void HandlePhaseTransitionTimeout();
    void FinishEntrance();
    void HandleEntranceTimeout();
    void PlayEntranceMontage();
    void StopEntranceMontage();
    void HandleEntranceMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void PlayPhaseTransitionMontage();
    void StopPhaseTransitionMontage();
    void HandleTransitionMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void LockTransitionMovement();
    void ReleaseTransitionMovement(bool bRestore);
    UBossDataAsset* GetBossData() const;
    UAnimMontage* GetPhaseTransitionMontage() const;
    UAnimMontage* GetEntranceMontage() const;
    const FBossPhaseSettings* GetPhaseSettings(EBossPhase Phase) const;
    void ApplyPhaseSettings(EBossPhase Phase);
    void InitializeDynamicMaterials();
    void ApplyPhaseVisuals(const FBossPhaseSettings& Settings);

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Boss", meta = (AllowPrivateAccess = "true"))
    EBossPhase CurrentPhase = EBossPhase::Phase1;

    // Fallback for missing callbacks or looping montages. Extended for long montages.
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase", meta = (ClampMin = "0.1", Units = "s"))
    float PhaseTransitionTimeout = 10.0f;

    TWeakObjectPtr<UAnimInstance> EntranceAnimInstance;
    TWeakObjectPtr<UAnimInstance> TransitionAnimInstance;
    TWeakObjectPtr<UCharacterMovementComponent> TransitionMovement;
    TWeakObjectPtr<UBrainComponent> TransitionBrain;
    TEnumAsByte<EMovementMode> PreviousMovementMode = MOVE_None;
    uint8 PreviousCustomMovementMode = 0;

    FTimerHandle PhaseTransitionTimerHandle;

    FTimerHandle EntranceTimerHandle;
	FTimerHandle HealthBarCheckTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> BossStatComponent;

	double LastDamageTime = -1.0;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> BodyDynamicMaterials;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> FurDynamicMaterials;

    bool bPhase2Triggered = false;
    bool bEncounterStarted = false;
    bool bEntrancePlaying = false;
};
