#include "Widget/BossHealthBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBossHealthBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// A native fallback makes the feature usable even before a styled Widget Blueprint is assigned.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BossHealthLayout"));
		if (UCanvasPanelSlot* LayoutSlot = Root->AddChildToCanvas(Layout))
		{
			LayoutSlot->SetAnchors(FAnchors(0.5f, 0.0f));
			LayoutSlot->SetAlignment(FVector2D(0.5f, 0.0f));
			LayoutSlot->SetPosition(FVector2D(0.0f, 42.0f));
			LayoutSlot->SetSize(FVector2D(640.0f, 64.0f));
		}

		MonsterNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MonsterNameText"));
		MonsterNameText->SetJustification(ETextJustify::Center);
		MonsterNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		Layout->AddChildToVerticalBox(MonsterNameText);

		HealthProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthProgressBar"));
		HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.72f, 0.03f, 0.03f, 1.0f));
		if (UVerticalBoxSlot* HealthSlot = Layout->AddChildToVerticalBox(HealthProgressBar))
		{
			HealthSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
			HealthSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
	}
}

void UBossHealthBarWidget::SetBossInfo(const FText& BossName, float CurrentHealth, float MaxHealth)
{
	if (IsValid(MonsterNameText))
	{
		MonsterNameText->SetText(BossName);
	}

	if (IsValid(HealthProgressBar))
	{
		TargetHealthPercent = MaxHealth > UE_SMALL_NUMBER
			? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
			: 0.0f;
		if (!bHealthInitialized)
		{
			DisplayedHealthPercent = TargetHealthPercent;
			bHealthInitialized = true;
		}
		HealthProgressBar->SetPercent(DisplayedHealthPercent);
	}
}

void UBossHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bHealthInitialized || !IsValid(HealthProgressBar)) return;

	DisplayedHealthPercent = HealthInterpolationSpeed <= 0.0f
		? TargetHealthPercent
		: FMath::FInterpTo(
			DisplayedHealthPercent, TargetHealthPercent, InDeltaTime, HealthInterpolationSpeed);
	if (FMath::IsNearlyEqual(DisplayedHealthPercent, TargetHealthPercent, 0.001f))
	{
		DisplayedHealthPercent = TargetHealthPercent;
	}
	HealthProgressBar->SetPercent(DisplayedHealthPercent);
}
