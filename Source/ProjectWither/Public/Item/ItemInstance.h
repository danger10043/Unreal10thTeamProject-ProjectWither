#pragma once

#include "CoreMinimal.h"
#include "ItemInstance.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType)
struct PROJECTWITHER_API FItemInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance")		// 원본 아이템 데이터
	TObjectPtr<UItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))	// 아이템 보유 수량
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))	// 현재 강화 수치 
	int32 EnhanceLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))	// 무기가 총일 경우 현재 장전된 탄약 수
	int32 CurrentAmmo = 0;
};