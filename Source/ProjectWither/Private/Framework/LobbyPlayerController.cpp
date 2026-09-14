#include "Framework/LobbyPlayerController.h"

#include "Widget/LobbyUIWidget.h"

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (!LobbyUIClass)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("LobbyPlayerController: LobbyUIClass is not assigned."));
        return;
    }

    LobbyUIInstance = CreateWidget<ULobbyUIWidget>(this, LobbyUIClass);

    if (!IsValid(LobbyUIInstance))
    {
        return;
    }

    LobbyUIInstance->SetIsFocusable(true);
    LobbyUIInstance->AddToViewport();

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(LobbyUIInstance->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock);

    SetInputMode(InputMode);
    bShowMouseCursor = true;
}