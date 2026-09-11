#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MonsterWorldHealthBarWidget.generated.h"

class UProgressBar;

/** Minimal overhead health widget used by normal monsters. */
UCLASS()
class PROJECTWITHER_API UMonsterWorldHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Monster|UI")
	void SetHealth(float CurrentHealth, float MaxHealth);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// The Widget Blueprint progress bar must use this name.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|UI", meta = (ClampMin = "0.0"))
	float HealthInterpolationSpeed = 5.0f;

private:
	float DisplayedHealthPercent = 1.0f;
	float TargetHealthPercent = 1.0f;
	bool bHealthInitialized = false;
};
