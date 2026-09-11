// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/StatUpgradeNPC.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerCharacter.h"
#include "Widget/StatUpgradeWindowWidget.h"

AStatUpgradeNPC::AStatUpgradeNPC()
{
	NPCName = FText::FromString(TEXT("스탯 강화 NPC"));
}

void AStatUpgradeNPC::HandleInteraction(AActor* Interactor)
{
	if (!IsValid(StatUpgradeWindowClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("AStatUpgradeNPC::HandleInteraction - StatUpgradeWindowClass가 지정되지 않았습니다."));
		return;
	}

	APawn* InteractingPawn = Cast<APawn>(Interactor);

	if (!IsValid(InteractingPawn) || !InteractingPawn->IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(InteractingPawn->GetController());

	if (!IsValid(PlayerController))
	{
		return;
	}

	FacePlayer(InteractingPawn);
	PlayGreetingMontage();

	if (!IsValid(StatUpgradeWindowInstance))
	{
		StatUpgradeWindowInstance = CreateWidget<UStatUpgradeWindowWidget>(
			PlayerController,
			StatUpgradeWindowClass);
	}

	if (!IsValid(StatUpgradeWindowInstance))
	{
		return;
	}

	StatUpgradeWindowInstance->SetOwningNPC(this);

	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InteractingPawn))
	{
		PlayerCharacter->SetCanMove(false);
	}

	PlayerController->FlushPressedKeys();

	if (!StatUpgradeWindowInstance->IsInViewport())
	{
		StatUpgradeWindowInstance->AddToViewport();
	}

	StatUpgradeWindowInstance->RefreshUpgradeInfo();

	PlayerController->bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(StatUpgradeWindowInstance->TakeWidget());
	PlayerController->SetInputMode(InputMode);
}

void AStatUpgradeNPC::FacePlayer(const AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return;
	}

	FVector ToInteractor = Interactor->GetActorLocation() - GetActorLocation();
	ToInteractor.Z = 0.0f;

	if (ToInteractor.IsNearlyZero())
	{
		return;
	}

	SetActorRotation(FRotator(0.0f, ToInteractor.Rotation().Yaw, 0.0f));
}

void AStatUpgradeNPC::PlayGreetingMontage()
{
	if (!IsValid(GreetingMontage) || !IsValid(NPCMesh))
	{
		return;
	}

	UAnimInstance* AnimInstance = NPCMesh->GetAnimInstance();

	if (!IsValid(AnimInstance))
	{
		return;
	}

	AnimInstance->Montage_Play(GreetingMontage);
	AnimInstance->Montage_JumpToSection(GreetingStartSection, GreetingMontage);
}

void AStatUpgradeNPC::EndGreetingMontage()
{
	if (!IsValid(GreetingMontage) || !IsValid(NPCMesh))
	{
		return;
	}

	UAnimInstance* AnimInstance = NPCMesh->GetAnimInstance();

	if (!IsValid(AnimInstance) || !AnimInstance->Montage_IsPlaying(GreetingMontage))
	{
		return;
	}

	AnimInstance->Montage_JumpToSection(GreetingEndSection, GreetingMontage);
}
