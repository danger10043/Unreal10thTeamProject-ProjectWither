#include "Widget/LobbyButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void ULobbyButtonWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (IsValid(LobbyButton))
    {
        LobbyButton->OnClicked.AddUniqueDynamic(
            this,
            &ULobbyButtonWidget::HandleButtonClicked);
    }
}

void ULobbyButtonWidget::SetButtonText(const FText& InText)
{
    if (IsValid(LobbyButtonText))
    {
        LobbyButtonText->SetText(InText);
    }
}

void ULobbyButtonWidget::HandleButtonClicked()
{
    OnClicked.Broadcast();
}