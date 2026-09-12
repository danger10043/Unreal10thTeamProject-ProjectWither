#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PotionCountWidget.generated.h"

class UInventoryComponent;
class UImage;
class UTextBlock;

UCLASS()
class PROJECTWITHER_API UPotionCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Potion")
	void SetPotionCount(int32 CurrentQuantity);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentPotionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PotionImage;

private:
	void BindInventory();
	void UnbindInventory();
	void BuildDefaultLayout();

	UFUNCTION()
	void HandlePotionChanged(int32 CurrentQuantity, int32 MaxQuantity);

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;
};
