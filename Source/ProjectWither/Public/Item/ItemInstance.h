#pragma once

#include "CoreMinimal.h"
#include "ItemInstance.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType)
struct PROJECTWITHER_API FItemInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance")
	TObjectPtr<UItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))
	int32 EnhanceLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Instance", meta = (ClampMin = "0"))
	int32 CurrentAmmo = 0;
};