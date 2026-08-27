// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DataAsset/ObjectPoolDataAsset.h"
#include "ObjectPoolSettings.generated.h"

/**
 * 
 */

// Config = Game : 설정창의 Game 카테고리에 저장(Config/DefaultGame.ini)
// DefaultConfig : 이 클래스 값이 기본 설정(프로젝트 설정 원본 = Config 폴더에 있는 ini에 저장된다.)
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Object Pool Settings"))
class PROJECTWITHER_API UObjectPoolSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Config : 이 프로퍼티는 ini 파일에 저장되고 불러와질 수 있다.
	// TSoftClassPtr : 타입에 대한 소프트 레퍼런스
	//UPROPERTY(Config, EditAnywhere, Category = "Pool Type")
	//TSoftClassPtr<AActor> DamagePopupClass = nullptr;

	UPROPERTY(Config, EditAnywhere, Category = "Pool Type")
	TArray<TSoftObjectPtr<UObjectPoolDataAsset>> PoolDataAssets;
};
