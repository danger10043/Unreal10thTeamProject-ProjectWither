// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonHeader/PlayerActionStateEnums.h"
#include "CurrentStateWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTWITHER_API UCurrentStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Player State")
	void SetPlayerActionState(EPlayerActionState NewState);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerStateText;
};
