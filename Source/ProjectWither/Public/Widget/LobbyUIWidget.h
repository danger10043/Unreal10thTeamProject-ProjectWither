#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUIWidget.generated.h"

class ULobbyButtonWidget;
class UWorld;

UCLASS()
class PROJECTWITHER_API ULobbyUIWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<ULobbyButtonWidget> WBP_StartButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<ULobbyButtonWidget> WBP_EndButton;

    // WBP_LobbyUI의 Class Defaults에서 실제 게임 레벨을 선택합니다.
    UPROPERTY(EditDefaultsOnly, Category = "UI|Lobby")
    TSoftObjectPtr<UWorld> GameLevel;

private:
    UFUNCTION()
    void HandleStartClicked();

    UFUNCTION()
    void HandleEndClicked();

    bool bStartingGame = false;
};