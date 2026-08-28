// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

class UItemDataAsset;

UCLASS()
class PROJECTWITHER_API APickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

	void InitializePickup(UItemDataAsset* InItemData, int32 InQuantity);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UItemDataAsset> ItemData = nullptr;

	UPROPERTY(VisibleInstanceOnly)
	int32 Quantity = 0;
};
