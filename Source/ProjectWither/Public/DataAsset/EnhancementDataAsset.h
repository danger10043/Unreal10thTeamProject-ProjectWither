// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset/CraftingRecipeDataAsset.h"
#include "EnhancementDataAsset.generated.h"

/*
 * 특정 강화 단계에 필요한 비용
 *
 * 예:
 * TargetLevel이 1이면 +0 장비를 +1로 만들 때 필요한 비용이다.
 */
USTRUCT(BlueprintType)
struct PROJECTWITHER_API FEnhancementLevelCost
{
	GENERATED_BODY()

public:
	// 강화 성공 후 도달할 단계
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "1", ClampMax = "10"))
	int32 TargetLevel = 1;

	// 해당 강화에 필요한 재료
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	TArray<FCraftingIngredient> RequiredMaterials;

	// 해당 강화에 필요한 골드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "0"))
	int32 GoldCost = 0;
};

/*
 * +1부터 +10까지의 강화 비용을 저장하는 데이터 에셋
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UEnhancementDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 장비가 도달할 수 있는 최대 강화 단계
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxEnhanceLevel = 10;

	// 강화 단계별 비용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement")
	TArray<FEnhancementLevelCost> LevelCosts;

	// 블루프린트에서 특정 단계의 강화 비용을 가져오는 함수
	UFUNCTION(BlueprintPure, Category = "Enhancement")
	bool GetCostForTargetLevel(int32 TargetLevel, FEnhancementLevelCost& OutCost) const;

	// C++에서 특정 단계의 강화 비용을 찾는 함수
	const FEnhancementLevelCost* FindCostForTargetLevel(int32 TargetLevel) const;
	
};
