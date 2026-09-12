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
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerCharacter.h"

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
	if (bEncounterStarted || CurrentPhase == EBossPhase::Dead) return;
	bEncounterStarted = true;
	SetPhase(EBossPhase::Phase1);
	OnBossEncounterStarted.Broadcast();
	PlayEntranceMontage();
}

void UBossComponent::RestartEncounterFromPool()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EntranceTimerHandle);
		World->GetTimerManager().ClearTimer(PhaseTransitionTimerHandle);
		World->GetTimerManager().ClearTimer(HealthBarCheckTimerHandle);
	}

	EntranceAnimInstance.Reset();
	TransitionAnimInstance.Reset();
	bEncounterStarted = false;
	bEntrancePlaying = false;
	bPhase2Triggered = false;
	LastDamageTime = -1.0;
	HideHealthBar();
	CurrentPhase = EBossPhase::Phase1;
	ReleaseTransitionMovement(true);
	ApplyPhaseSettings(EBossPhase::Phase1);
	StartEncounter();
}

void UBossComponent::PrepareForPoolReturn()
{
	HideHealthBar();
	bEntrancePlaying = false;
	StopEntranceMontage();
	StopPhaseTransitionMontage();
	ReleaseTransitionMovement(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EntranceTimerHandle);
		World->GetTimerManager().ClearTimer(PhaseTransitionTimerHandle);
		World->GetTimerManager().ClearTimer(HealthBarCheckTimerHandle);
	}
}

void UBossComponent::PlayEntranceMontage()
{
	UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>();
	UAnimMontage* Montage = GetEntranceMontage();
	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (!IsValid(Montage) || !IsValid(AnimInstance)) return;

	bEntrancePlaying = true;
	if (IsValid(Monster))
	{
		Monster->SetCombatLocked(true);
		Monster->SetDamageLocked(true);
		Monster->CancelAttack();
		Monster->FinishAttack();
		Monster->CancelSearch();
	}
	LockTransitionMovement();

	const float Duration = AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::Duration);
	if (Duration <= 0.0f)
	{
		FinishEntrance();
		return;
	}

	EntranceAnimInstance = AnimInstance;
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UBossComponent::HandleEntranceMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(EntranceTimerHandle, this,
			&UBossComponent::HandleEntranceTimeout,
			FMath::Max(PhaseTransitionTimeout, Duration + 1.0f), false);
	}
}

void UBossComponent::HandleEntranceMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != GetEntranceMontage()) return;
	EntranceAnimInstance.Reset();
	FinishEntrance();
}

void UBossComponent::HandleEntranceTimeout()
{
	FinishEntrance();
}

void UBossComponent::FinishEntrance()
{
	if (!bEntrancePlaying) return;
	bEntrancePlaying = false;
	StopEntranceMontage();
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(EntranceTimerHandle);

	UMonsterComponent* Monster = GetOwner()->FindComponentByClass<UMonsterComponent>();
	if (IsValid(Monster) && !Monster->IsDead())
	{
		Monster->SetCombatLocked(false);
		Monster->SetDamageLocked(false);
	}
	ReleaseTransitionMovement(IsValid(Monster) && !Monster->IsDead());
}

void UBossComponent::StopEntranceMontage()
{
	UAnimMontage* Montage = GetEntranceMontage();
	UAnimInstance* AnimInstance = EntranceAnimInstance.Get();
	EntranceAnimInstance.Reset();
	if (IsValid(AnimInstance) && IsValid(Montage))
	{
		FOnMontageEnded EmptyDelegate;
		AnimInstance->Montage_SetEndDelegate(EmptyDelegate, Montage);
		AnimInstance->Montage_Stop(0.0f, Montage);
	}
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
	FinishPhaseTransition();
}

void UBossComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopEntranceMontage();
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
		World->GetTimerManager().ClearTimer(EntranceTimerHandle);
		World->GetTimerManager().ClearTimer(HealthBarCheckTimerHandle);
	}
	if (IsValid(BossStatComponent))
	{
		BossStatComponent->OnHealthChanged.RemoveDynamic(this, &UBossComponent::HandleHealthChanged);
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
			BossStatComponent = Stat;
            Stat->OnHealthChanged.AddUniqueDynamic(
                this,
                &UBossComponent::HandleHealthChanged
			);
			RefreshHealthBar(Stat->GetCurrentHealth(), Stat->GetMaxHealth());
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

	InitializeDynamicMaterials();
	ApplyPhaseSettings(EBossPhase::Phase1);
}

void UBossComponent::HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	RefreshHealthBar(CurrentHealth, MaxHealth);
	if (ChangedAmount < 0.0f && CurrentHealth > 0.0f)
	{
		ShowHealthBar();
	}

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
	HideHealthBar();
	bEntrancePlaying = false;
	StopEntranceMontage();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EntranceTimerHandle);
	}
	ReleaseTransitionMovement(false);
	SetPhase(EBossPhase::Dead);
	OnBossEncounterEnded.Broadcast();
}

void UBossComponent::RefreshHealthBar(float CurrentHealth, float MaxHealth)
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = IsValid(World) ? World->GetFirstPlayerController() : nullptr;
	if (APlayerCharacter* Player = IsValid(PlayerController)
		? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr)
	{
		Player->UpdateBossHealthBar(GetOwner(), CurrentHealth, MaxHealth);
	}
}

void UBossComponent::ShowHealthBar()
{
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	LastDamageTime = World->GetTimeSeconds();
	APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn());
	if (!IsValid(Player) || !Player->ShowBossHealthBar(GetOwner()))
	{
		return;
	}
	World->GetTimerManager().SetTimer(
		HealthBarCheckTimerHandle, this, &UBossComponent::CheckHealthBarVisibility, 0.2f, true);
}

void UBossComponent::HideHealthBar()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HealthBarCheckTimerHandle);
	}
	UWorld* World = GetWorld();
	APlayerController* PlayerController = IsValid(World) ? World->GetFirstPlayerController() : nullptr;
	if (APlayerCharacter* Player = IsValid(PlayerController)
		? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr)
	{
		Player->HideBossHealthBar(GetOwner());
	}
}

FText UBossComponent::GetBossDisplayName() const
{
	const UBossDataAsset* BossData = GetBossData();
	return IsValid(BossData) ? BossData->MonsterName : FText::GetEmpty();
}

void UBossComponent::CheckHealthBarVisibility()
{
	UWorld* World = GetWorld();
	const UBossDataAsset* BossData = GetBossData();
	APlayerController* PlayerController = IsValid(World) ? World->GetFirstPlayerController() : nullptr;
	APawn* PlayerPawn = IsValid(PlayerController) ? PlayerController->GetPawn() : nullptr;
	if (!IsValid(World) || !IsValid(BossData) || !IsValid(PlayerPawn) || !IsValid(GetOwner()))
	{
		HideHealthBar();
		return;
	}

	const bool bTimedOut = BossData->HealthBarInactiveTime <= 0.0f ||
		World->GetTimeSeconds() - LastDamageTime >= BossData->HealthBarInactiveTime;
	const bool bTooFar = BossData->HealthBarMaxDistance <= 0.0f ||
		FVector::DistSquared(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation()) >
		FMath::Square(BossData->HealthBarMaxDistance);
	if (bTimedOut || bTooFar)
	{
		HideHealthBar();
	}
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
		FinishPhaseTransition();
		return;
	}

	const float Duration = AnimInstance->Montage_Play(
		TransitionMontage, 1.0f, EMontagePlayReturnType::Duration);
	if (Duration <= 0.0f)
	{
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

UAnimMontage* UBossComponent::GetEntranceMontage() const
{
	const UBossDataAsset* BossData = GetBossData();
	return IsValid(BossData) ? BossData->EntranceMontage.Get() : nullptr;
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
		const UBossDataAsset* BossData = GetBossData();
		Monster->SetAdditionalAttackMontage(
			Phase == EBossPhase::Phase2 && IsValid(BossData)
			? BossData->Phase2AttackMontage.Get()
			: nullptr,
			Phase == EBossPhase::Phase2 && IsValid(BossData) &&
			BossData->bUseOnlyPhase2AttackMontage);
	}

	if (UStatComponent* Stat = OwnerCharacter->FindComponentByClass<UStatComponent>())
	{
		Stat->SetCombatMultipliers(
			Settings->AttackPowerMultiplier,
			Settings->DefenseMultiplier);
	}

	ApplyPhaseVisuals(*Settings);
}

void UBossComponent::InitializeDynamicMaterials()
{
	BodyDynamicMaterials.Reset();
	FurDynamicMaterials.Reset();

	const UBossDataAsset* BossData = GetBossData();
	USkeletalMeshComponent* Mesh = IsValid(GetOwner())
		? GetOwner()->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	if (!IsValid(BossData) || !IsValid(Mesh)) return;

	auto CreateForSlots = [Mesh](const TArray<FName>& SlotNames,
		TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMaterials)
	{
		for (const FName SlotName : SlotNames)
		{
			const int32 MaterialIndex = Mesh->GetMaterialIndex(SlotName);
			if (MaterialIndex == INDEX_NONE) continue;

			if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(MaterialIndex))
			{
				OutMaterials.AddUnique(Material);
			}
		}
	};

	CreateForSlots(BossData->BodyMaterialSlots, BodyDynamicMaterials);
	CreateForSlots(BossData->FurMaterialSlots, FurDynamicMaterials);
}

void UBossComponent::ApplyPhaseVisuals(const FBossPhaseSettings& Settings)
{
	const UBossDataAsset* BossData = GetBossData();
	if (!IsValid(BossData)) return;

	for (UMaterialInstanceDynamic* Material : BodyDynamicMaterials)
	{
		if (!IsValid(Material)) continue;
		Material->SetVectorParameterValue(BossData->BodyTintParameterName, Settings.BodyTint);
		Material->SetScalarParameterValue(
			BossData->EmissiveStrengthParameterName,
			FMath::Max(0.0f, Settings.EmissiveStrength));
	}

	for (UMaterialInstanceDynamic* Material : FurDynamicMaterials)
	{
		if (!IsValid(Material)) continue;
		Material->SetVectorParameterValue(BossData->FurTintParameterName, Settings.FurTint);
		Material->SetScalarParameterValue(
			BossData->EmissiveStrengthParameterName,
			FMath::Max(0.0f, Settings.EmissiveStrength));
	}
}

void UBossComponent::LockTransitionMovement()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn)) return;

	if (UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
	{
		if (!TransitionMovement.IsValid())
		{
			TransitionMovement = Movement;
			PreviousMovementMode = Movement->MovementMode;
			PreviousCustomMovementMode = Movement->CustomMovementMode;
		}
		Movement->StopMovementImmediately();
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

	if (IsValid(Brain) && Brain->IsPaused())
	{
		Brain->ResumeLogic(TEXT("Boss phase transition finished"));
	}
}

