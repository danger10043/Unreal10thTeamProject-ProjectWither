#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairUI.generated.h"

class UImage;

UCLASS()
class PROJECTWITHER_API UCrosshairUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

private:
	void UpdateCrosshair(float DeltaTime);
	void SetCrosshairVisible(bool bVisible);
	void ApplyCrosshairGap();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CenterDot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TopBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BottomBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LeftBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RightBar;

	// 점 가장자리와 막대 안쪽 끝 사이의 간격
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair", meta = (ClampMin = "0.0"))
	float MinGap = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair", meta = (ClampMin = "0.0"))
	float MaxGap = 28.0f;

	// 최대 간격이 되는 수평 이동 속도(cm/s)
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair", meta = (ClampMin = "1.0"))
	float SpeedForMaxSpread = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Crosshair", meta = (ClampMin = "0.1"))
	float SpreadInterpSpeed = 10.0f;

private:
	float CurrentGap = 6.0f;
	bool bCrosshairVisible = false;
};
