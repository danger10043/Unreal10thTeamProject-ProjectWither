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
	// The Widget Blueprint progress bar must use this name.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;
};
