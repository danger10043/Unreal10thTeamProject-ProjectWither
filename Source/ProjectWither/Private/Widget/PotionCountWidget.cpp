#include "Widget/PotionCountWidget.h"

#include "Component/InventoryComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Texture2D.h"

void UPotionCountWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BuildDefaultLayout();
}

void UPotionCountWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindInventory();
}

void UPotionCountWidget::NativeDestruct()
{
	UnbindInventory();

	Super::NativeDestruct();
}

void UPotionCountWidget::SetPotionCount(int32 CurrentQuantity)
{
	const int32 SafeCurrentQuantity = FMath::Max(0, CurrentQuantity);

	if (IsValid(CurrentPotionText))
	{
		CurrentPotionText->SetText(FText::AsNumber(SafeCurrentQuantity));
	}
}

void UPotionCountWidget::BindInventory()
{
	UnbindInventory();

	const APawn* OwningPawn = GetOwningPlayerPawn();
	if (!IsValid(OwningPawn)) return;

	InventoryComponent = OwningPawn->FindComponentByClass<UInventoryComponent>();
	if (!IsValid(InventoryComponent)) return;

	InventoryComponent->OnPotionChanged.AddUniqueDynamic(
		this,
		&UPotionCountWidget::HandlePotionChanged
	);

	HandlePotionChanged(
		InventoryComponent->GetCurrentPotionQuantity(),
		InventoryComponent->GetMaxPotionQuantity()
	);
}

void UPotionCountWidget::UnbindInventory()
{
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnPotionChanged.RemoveDynamic(
			this,
			&UPotionCountWidget::HandlePotionChanged
		);
	}

	InventoryComponent = nullptr;
}

void UPotionCountWidget::HandlePotionChanged(int32 CurrentQuantity, int32 MaxQuantity)
{
	SetPotionCount(CurrentQuantity);
}

void UPotionCountWidget::BuildDefaultLayout()
{
	if (!IsValid(WidgetTree) || IsValid(WidgetTree->RootWidget)) return;

	USizeBox* RootSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PotionRoot"));
	RootSizeBox->SetWidthOverride(150.0f);
	RootSizeBox->SetHeightOverride(150.0f);
	WidgetTree->RootWidget = RootSizeBox;

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("PotionOverlay"));
	RootSizeBox->AddChild(Overlay);

	PotionImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("PotionImage"));
	if (UTexture2D* PotionTexture = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Game/Main/UI/MainUI/Textures/T_HealpackIcon_150.T_HealpackIcon_150")))
	{
		PotionImage->SetBrushFromTexture(PotionTexture, true);
	}
	Overlay->AddChildToOverlay(PotionImage);

	CurrentPotionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CurrentPotionText"));
	CurrentPotionText->SetText(FText::AsNumber(0));
	CurrentPotionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	CurrentPotionText->SetShadowOffset(FVector2D(2.0f, 2.0f));
	CurrentPotionText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));
	FSlateFontInfo FontInfo = CurrentPotionText->GetFont();
	FontInfo.Size = 32;
	CurrentPotionText->SetFont(FontInfo);

	UOverlaySlot* CountSlot = Overlay->AddChildToOverlay(CurrentPotionText);
	CountSlot->SetHorizontalAlignment(HAlign_Right);
	CountSlot->SetVerticalAlignment(VAlign_Bottom);
	CountSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 8.0f));
}
