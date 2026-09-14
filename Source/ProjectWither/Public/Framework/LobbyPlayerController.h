#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUIWidget;

UCLASS()
class PROJECTWITHER_API ALobbyPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Lobby")
    TSubclassOf<ULobbyUIWidget> LobbyUIClass;

private:
    UPROPERTY(Transient)
    TObjectPtr<ULobbyUIWidget> LobbyUIInstance;
};