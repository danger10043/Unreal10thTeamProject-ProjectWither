// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/ItemDataAsset.h"
#include "PotionDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UPotionDataAsset : public UItemDataAsset
{
	GENERATED_BODY()
	
public:
	UPotionDataAsset();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion", meta = (ClampMin = "0.0")) // 포션 사용 시 회복되는 체력량
	float HealAmount = 0.0f;

public:
	UFUNCTION(BlueprintPure, Category = "Potion") 
	float GetHealAmount() const;

};
