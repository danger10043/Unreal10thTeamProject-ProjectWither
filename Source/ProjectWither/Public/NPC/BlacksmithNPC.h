// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCBase.h"
#include "BlacksmithNPC.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTWITHER_API ABlacksmithNPC : public ANPCBase
{
	GENERATED_BODY()
	
public:
	ABlacksmithNPC();

protected:
	virtual void HandleInteraction(AActor* Interactor) override;
};