// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CommonHeader/ObjectPoolEnums.h"
#include "ObjectPoolDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTWITHER_API UObjectPoolDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 풀에서 생성하고 재사용할 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pool")
	TSoftClassPtr<AActor> ActorClass = nullptr;

	// 월드 시작 시 미리 생성할 액터 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (ClampMin = "0"))
	int32 InitialSize = 0;

	// Grow 이외의 정책에서 풀에 유지할 최대 액터 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (ClampMin = "1"))
	int32 MaxSize = 32;

	// 풀의 액터 수가 MaxSize에 도달했을 때 적용할 정책
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Pool")
	EObjectPoolPolicy MaxPolicy = EObjectPoolPolicy::Grow;
};
