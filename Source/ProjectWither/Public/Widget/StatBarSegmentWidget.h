#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "StatBarSegmentWidget.generated.h"

class UImage;
class USizeBox;

UCLASS()
class PROJECTWITHER_API UStatBarSegmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAppearance(const FSlateBrush& Brush, FVector2D Size);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SegmentSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SegmentImage;

private:
	void ApplyAppearance();

	UPROPERTY(Transient)
	FSlateBrush CurrentBrush;

	FVector2D CurrentSize = FVector2D(8.0f, 24.0f);
	bool bHasAppearance = false;
};