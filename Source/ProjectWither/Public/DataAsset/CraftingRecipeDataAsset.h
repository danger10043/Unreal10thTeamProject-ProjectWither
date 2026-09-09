// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CraftingRecipeDataAsset.generated.h"

class UItemDataAsset;

/**
 * 제작에 필요한 재료 하나
 *
 * 예:
 * 철광석 3개
 */
USTRUCT(BlueprintType)
struct PROJECTWITHER_API FCraftingIngredient
{
	GENERATED_BODY()
	
public:
	// 필요한 재료 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<UItemDataAsset> ItemData = nullptr;

	// 필요한 재료 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crafting", meta = (ClampMin = "1"))
	int32 Quantity = 1;

};

/**
 * 대장장이 제작 레시피 데이터
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UCraftingRecipeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 제작 결과로 지급할 장비
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Result")
	TObjectPtr<UItemDataAsset> ResultItem = nullptr;

	// 제작 결과 수량
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Result", meta = (ClampMin = "1"))
	int32 ResultQuantity = 1;

	// 제작에 필요한 재료 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Cost")
	TArray<FCraftingIngredient> RequiredMaterials;

	// 제작에 필요한 골드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Cost", meta = (ClampMin = "0"))
	int32 GoldCost = 0;
};