// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SavePointMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Framework/SubSystem/SavePointSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

void USavePointMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindButtons();
}

void USavePointMenuWidget::NativeDestruct()
{
	UnbindButtons();

	Super::NativeDestruct();
}

void USavePointMenuWidget::OpenAt(FName InCurrentSavePointId)
{
	CurrentSavePointId = InCurrentSavePointId;

	if (IsValid(RestedAtText))
	{
		FText SavePointDisplayName = FText::GetEmpty();

		if (const USavePointSubsystem* Subsystem = GetSavePointSubsystem())
		{
			for (const FSavePointInfo& Info : Subsystem->GetActivatedSavePoints())
			{
				if (Info.SavePointId == CurrentSavePointId)
				{
					SavePointDisplayName = Info.DisplayName;
					break;
				}
			}
		}

		RestedAtText->SetText(SavePointDisplayName);
	}

	OnMenuOpened();
}

TArray<FSavePointInfo> USavePointMenuWidget::GetTravelDestinations() const
{
	TArray<FSavePointInfo> Destinations;

	const USavePointSubsystem* Subsystem = GetSavePointSubsystem();

	if (!IsValid(Subsystem)) { return Destinations; }

	for (const FSavePointInfo& Info : Subsystem->GetActivatedSavePoints())
	{
		if (Info.SavePointId == CurrentSavePointId) { continue; }

		Destinations.Add(Info);
	}

	return Destinations;
}

bool USavePointMenuWidget::TravelTo(FName TargetSavePointId)
{
	if (TargetSavePointId == CurrentSavePointId) { return false; }

	USavePointSubsystem* Subsystem = GetSavePointSubsystem();

	if (!IsValid(Subsystem) || !Subsystem->IsSavePointActivated(TargetSavePointId)) { return false; }

	FTransform TargetTransform;

	if (!Subsystem->GetSavePointTransform(TargetSavePointId, TargetTransform)) { return false; }

	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningPlayerPawn());

	if (!IsValid(OwningCharacter)) { return false; }

	if (UCharacterMovementComponent* Movement = OwningCharacter->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	OwningCharacter->SetActorLocationAndRotation(
		TargetTransform.GetLocation(),
		TargetTransform.GetRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	// 목적지로 이동했으므로 그 세이브 포인트가 새로운 부활 지점이 된다.
	Subsystem->ActivateSavePoint(TargetSavePointId);

	CloseAndRestoreInput();

	return true;
}

void USavePointMenuWidget::BindButtons()
{
	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &USavePointMenuWidget::HandleCloseClicked);
	}
}

void USavePointMenuWidget::UnbindButtons()
{
	if (IsValid(CloseButton))
	{
		CloseButton->OnClicked.RemoveDynamic(this, &USavePointMenuWidget::HandleCloseClicked);
	}
}

void USavePointMenuWidget::CloseAndRestoreInput()
{
	RemoveFromParent();

	APlayerController* PlayerController = GetOwningPlayer();

	if (!IsValid(PlayerController)) { return; }

	PlayerController->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
}

USavePointSubsystem* USavePointMenuWidget::GetSavePointSubsystem() const
{
	const UWorld* World = GetWorld();
	UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;

	return IsValid(GameInstance) ? GameInstance->GetSubsystem<USavePointSubsystem>() : nullptr;
}

void USavePointMenuWidget::HandleCloseClicked()
{
	CloseAndRestoreInput();
}
