// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CommonHeader/ObjectPoolEnums.h"
#include "ObjectPoolDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTWITHER_API UObjectPoolDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 풀링 될 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> ActorClass = nullptr;

	// 사전 생성해 둘 초기 수량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 InitialSize = 0;

	// 풀에서 최대로 관리 가능한 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 MaxSize = 0;

	// 최대치일 때 생성 정책
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EObjectPoolPolicy MaxPolicy = EObjectPoolPolicy::Grow;
};
