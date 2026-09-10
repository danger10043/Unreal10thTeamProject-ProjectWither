// + + < 코드 추가 시작 > + +
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "SegmentedStatBarWidget.generated.h"

class UImage;
class UHorizontalBox;
class UStatBarSegmentWidget;

UCLASS()
class PROJECTWITHER_API USegmentedStatBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Stat Bar")
	void SetValues(float CurrentValue, float MaxValue);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> SegmentContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> StatTypeTextImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stat Bar")
	TSubclassOf<UStatBarSegmentWidget> SegmentWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stat Bar")
	FSlateBrush BarOn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stat Bar")
	FSlateBrush BarOff;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stat Bar")
	FVector2D SegmentSize = FVector2D(10.0f, 48.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stat Bar",
		meta = (ClampMin = "0.0"))
	float SegmentSpacing = 2.0f;

	UPROPERTY(EditAnywhere, Category = "UI|Stat Bar|Preview",
		meta = (ClampMin = "0.0"))
	float PreviewCurrentValue = 35.0f;

	UPROPERTY(EditAnywhere, Category = "UI|Stat Bar|Preview",
		meta = (ClampMin = "0.0"))
	float PreviewMaxValue = 50.0f;

private:
	void RefreshSegments(bool bForceRebuild);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStatBarSegmentWidget>> Segments;

	float CachedCurrentValue = 0.0f;
	float CachedMaxValue = 0.0f;

	static constexpr float ValuePerSegment = 5.0f;
};
// + + < 코드 추가 종료 > + +