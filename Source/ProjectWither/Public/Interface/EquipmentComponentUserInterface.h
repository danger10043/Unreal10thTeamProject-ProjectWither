#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EquipmentComponentUserInterface.generated.h"

class UEquipmentComponent;

UINTERFACE(MinimalAPI)
class UEquipmentComponentUserInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTWITHER_API IEquipmentComponentUserInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UEquipmentComponent* GetEquipmentComponent() const;
};
