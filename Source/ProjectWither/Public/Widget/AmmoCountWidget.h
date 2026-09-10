#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoCountWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTWITHER_API UAmmoCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Ammo")
	void SetAmmo(int32 CurrentAmmo, int32 MaxAmmo);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentAmmoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxAmmoText;
};