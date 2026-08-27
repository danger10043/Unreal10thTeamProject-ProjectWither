// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonHeader/ArmorTypeEnums.h"
#include "DataAsset/ItemDataAsset.h"
#include "ArmorDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UArmorDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:
	UArmorDataAsset();

protected:
	// 방어구가 장착되는 부위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
	EArmorType ArmorType = EArmorType::Helmet;

	// 방어구가 제공하는 기본 방어력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor", meta = (ClampMin = "0.0"))
	float ArmorDefense = 0.0f;

public:
	UFUNCTION(BlueprintPure, Category = "Armor")
	EArmorType GetArmorType() const;

	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetArmorDefense() const;
};
