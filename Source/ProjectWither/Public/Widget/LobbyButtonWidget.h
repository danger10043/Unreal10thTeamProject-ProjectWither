#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLobbyButtonClicked);

UCLASS()
class PROJECTWITHER_API ULobbyButtonWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "UI|Lobby")
    FLobbyButtonClicked OnClicked;

    void SetButtonText(const FText& InText);

protected:
    virtual void NativeOnInitialized() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> LobbyButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> LobbyButtonText;

private:
    UFUNCTION()
    void HandleButtonClicked();
};