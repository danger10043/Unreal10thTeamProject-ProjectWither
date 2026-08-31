#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StatComponentUserInterface.generated.h"

class UStatComponent;

UINTERFACE(MinimalAPI)
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stat")
	UStatComponent* GetStatComponent() const;
};
