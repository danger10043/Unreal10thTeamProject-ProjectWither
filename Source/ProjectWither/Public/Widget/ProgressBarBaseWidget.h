// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProgressBarBaseWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECTWITHER_API UProgressBarBaseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI|Progress Bar")
	void SetValues(float CurrentValue, float MaxValue);

	UFUNCTION(BlueprintCallable, Category = "UI|Progress Bar")
	void SetBarColors(FLinearColor BackgroundColor, FLinearColor FillColor);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> TargetProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentValueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxValueText;
};
