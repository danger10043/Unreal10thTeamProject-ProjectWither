#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StatComponentUserInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UStatComponentUserInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTWITHER_API IStatComponentUserInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Stat")
	virtual UStatComponent* GetStatComponent() const = 0;
};
