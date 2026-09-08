// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/StatUpgradeNPC.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
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

	const APawn* InteractingPawn = Cast<APawn>(Interactor);

	if (!IsValid(InteractingPawn) || !InteractingPawn->IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(InteractingPawn->GetController());

	if (!IsValid(PlayerController))
	{
		return;
	}

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
