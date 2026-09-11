#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/** Screen-fixed boss name and health bar. May be used directly or as a Widget Blueprint parent. */
UCLASS()
class PROJECTWITHER_API UBossHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	void SetBossInfo(const FText& BossName, float CurrentHealth, float MaxHealth);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MonsterNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|UI", meta = (ClampMin = "0.0"))
	float HealthInterpolationSpeed = 5.0f;

private:
	float DisplayedHealthPercent = 1.0f;
	float TargetHealthPercent = 1.0f;
	bool bHealthInitialized = false;
};
