// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SavePointMenuWidget.h"

#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Framework/SubSystem/SavePointSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerCharacter.h"

void USavePointMenuWidget::NativeConstruct()
{
	SetIsFocusable(true);

	Super::NativeConstruct();

	BindButtons();
}

void USavePointMenuWidget::NativeDestruct()
{
	UnbindButtons();

	Super::NativeDestruct();
}

FReply USavePointMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		if (!InKeyEvent.IsRepeat())
		{
			CloseAndRestoreInput();
		}

		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void USavePointMenuWidget::OpenAt(FName InCurrentSavePointId)
{
	CurrentSavePointId = InCurrentSavePointId;

	FSavePointInfo CurrentInfo;

	const USavePointSubsystem* Subsystem = GetSavePointSubsystem();

	if (IsValid(Subsystem))
	{
		Subsystem->GetSavePointInfo(CurrentSavePointId, CurrentInfo);
	}

	if (IsValid(RestedAtText))
	{
		RestedAtText->SetText(CurrentInfo.DisplayName);
	}

	// 목록에서 마우스가 벗어나면 다시 현재 위치한 세이브 포인트의 이미지로 돌아온다.
	DefaultPreviewImage = CurrentInfo.LocationImage;
	SetPreviewImage(DefaultPreviewImage);

	OnMenuOpened();
}

void USavePointMenuWidget::SetPreviewImage(UTexture2D* Image)
{
	if (!IsValid(PreviewImage)) { return; }

	if (IsValid(Image))
	{
		PreviewImage->SetBrushFromTexture(Image);
		PreviewImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		PreviewImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USavePointMenuWidget::RestorePreviewImage()
{
	SetPreviewImage(DefaultPreviewImage);
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
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn()))
	{
		PlayerCharacter->SetCanMove(true);
	}

	RemoveFromParent();

	APlayerController* PlayerController = GetOwningPlayer();

	if (!IsValid(PlayerController)) { return; }

	PlayerController->FlushPressedKeys();
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
