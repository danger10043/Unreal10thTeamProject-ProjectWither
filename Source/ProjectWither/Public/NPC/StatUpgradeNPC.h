// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCBase.h"
#include "StatUpgradeNPC.generated.h"

class UStatUpgradeWindowWidget;

UCLASS()
class PROJECTWITHER_API AStatUpgradeNPC : public ANPCBase
{
	GENERATED_BODY()

public:
	AStatUpgradeNPC();

protected:
	virtual void HandleInteraction(AActor* Interactor) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UStatUpgradeWindowWidget> StatUpgradeWindowClass;

	UPROPERTY(Transient)
	TObjectPtr<UStatUpgradeWindowWidget> StatUpgradeWindowInstance;
};
