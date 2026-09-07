// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class AActor;

UINTERFACE(BlueprintType, Blueprintable)
class PROJECTWITHER_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTWITHER_API IInteractableInterface
{
	GENERATED_BODY()

public:
	// 이 플레이어가 현재 상호작용할 수 있는지 확인
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Intraction")
	bool CanInteract(AActor* Interactor) const;

	// 상호작용 시작 요청
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Intraction")
	void Interact(AActor* Interactor);

};
