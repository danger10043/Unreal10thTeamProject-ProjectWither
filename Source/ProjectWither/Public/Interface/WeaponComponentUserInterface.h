#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponComponentUserInterface.generated.h"

class UWeaponComponent;

UINTERFACE(MinimalAPI)
class UWeaponComponentUserInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTWITHER_API IWeaponComponentUserInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon")
	UWeaponComponent* GetWeaponComponent() const;
};
