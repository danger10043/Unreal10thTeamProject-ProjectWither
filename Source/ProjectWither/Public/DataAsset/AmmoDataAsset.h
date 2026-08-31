// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonHeader/AmmoTypeEnums.h"
#include "DataAsset/ItemDataAsset.h"
#include "AmmoDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UAmmoDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:
	UAmmoDataAsset();

protected:
	// 이 탄약 아이템이 어떤 종류의 총기에 사용되는지 구분
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	EAmmoType AmmoType = EAmmoType::Normal;

public:
	UFUNCTION(BlueprintPure, Category = "Ammo")
	EAmmoType GetAmmoType() const;
};
