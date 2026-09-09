// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCBase.h"
#include "BlacksmithNPC.generated.h"

class UUserWidget;
class UBlacksmithComponent;

/**
 * 
 */
UCLASS()
class PROJECTWITHER_API ABlacksmithNPC : public ANPCBase
{
	GENERATED_BODY()
	
public:
	ABlacksmithNPC();

	UFUNCTION(BlueprintPure, Category = "Blacksmith")
	UBlacksmithComponent* GetBlacksmithComponent() const { return BlacksmithComponent; }

protected:
	virtual void HandleInteraction(AActor* Interactor) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|UI")
	TSubclassOf<UUserWidget> BlacksmithWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	TObjectPtr<UBlacksmithComponent> BlacksmithComponent;

};