// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CommonHeader/ItemTypeEnums.h"
#include "ItemDataAsset.generated.h"

class UEnhancementDataAsset;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTWITHER_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 ItemId = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EItemType ItemType = EItemType::Weapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
	int32 Price = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> ItemImage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMesh> ItemMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item" , meta = (ClampMin = "1") )
	int32 MaxStack = 1;

	// 이 아이템 전용 강화 비용 정보, 설정하지 않으면 대장장이의 기본 강화 데이터를 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Enhancement")
	TObjectPtr<UEnhancementDataAsset> EnhancementProfile = nullptr;

public:
	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetItemId() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetItemName() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	FText GetDescription() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	EItemType GetItemType() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetPrice() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	UTexture2D* GetItemImage() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	UStaticMesh* GetItemMesh() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	int32 GetMaxStack() const;

	UFUNCTION(BlueprintPure, Category = "Item|Enhancement")
	UEnhancementDataAsset* GetEnhancementProfile() const;
};
