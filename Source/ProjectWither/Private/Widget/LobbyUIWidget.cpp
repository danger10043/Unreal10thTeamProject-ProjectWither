#include "Widget/LobbyUIWidget.h"

#include "Widget/LobbyButtonWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void ULobbyUIWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (IsValid(WBP_StartButton))
    {
        WBP_StartButton->OnClicked.AddUniqueDynamic(
            this,
            &ULobbyUIWidget::HandleStartClicked);
    }

    if (IsValid(WBP_EndButton))
    {
        WBP_EndButton->OnClicked.AddUniqueDynamic(
            this,
            &ULobbyUIWidget::HandleEndClicked);
    }
}

void ULobbyUIWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(WBP_StartButton))
    {
        WBP_StartButton->SetButtonText(
            NSLOCTEXT("LobbyUI", "StartGame", "게임 시작"));
    }

    if (IsValid(WBP_EndButton))
    {
        WBP_EndButton->SetButtonText(
            NSLOCTEXT("LobbyUI", "EndGame", "게임 종료"));
    }
}

void ULobbyUIWidget::HandleStartClicked()
{
    if (bStartingGame)
    {
        return;
    }

    if (GameLevel.IsNull())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("LobbyUI: GameLevel is not assigned."));
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();

    if (!IsValid(PlayerController))
    {
        return;
    }

    bStartingGame = true;

    PlayerController->SetInputMode(FInputModeGameOnly());
    PlayerController->bShowMouseCursor = false;

    RemoveFromParent();

    UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameLevel);
}

void ULobbyUIWidget::HandleEndClicked()
{
    UKismetSystemLibrary::QuitGame(
        this,
        GetOwningPlayer(),
        EQuitPreference::Quit,
        false);
}