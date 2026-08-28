#pragma once

#include "CoreMinimal.h"
#include "MonsterDropData.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType)
struct PROJECTWITHER_API FMonsterDropData
{
public:
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<UItemDataAsset> ItemData = nullptr; // 드랍 가능한 아이템

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropRate;	// 드랍 확률 0.0 ~ 1.0

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MinQuantity = 1; // 최소 드랍 개수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1; // 최대 드랍 개수
};