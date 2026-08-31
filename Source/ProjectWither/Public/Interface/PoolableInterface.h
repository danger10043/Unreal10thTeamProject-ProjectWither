// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableInterface.generated.h"

UINTERFACE(BlueprintType)
class PROJECTWITHER_API UPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTWITHER_API IPoolableInterface
{
	GENERATED_BODY()

public:
	// 액터가 풀에서 획득되어 활성화된 뒤 액터별 상태를 초기화
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnSpawnFromPool();

	// 액터가 비활성화되어 풀로 반환될 때 액터별 상태를 정리
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnReturnToPool();
};
