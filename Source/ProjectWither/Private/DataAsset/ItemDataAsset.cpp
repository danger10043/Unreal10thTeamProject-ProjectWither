// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/ItemDataAsset.h"


int32 UItemDataAsset::GetItemId() const		// ItemId를 반환하는 함수
{
	return ItemId;
}

FText UItemDataAsset::GetItemName() const	// ItemName을 반환하는 함수
{
	return ItemName;
}

FText UItemDataAsset::GetDescription() const	// Description을 반환하는 함수
{
	return Description;
}

EItemType UItemDataAsset::GetItemType() const	// ItemType를 반환하는 함수
{
	return ItemType;
}

int32 UItemDataAsset::GetPrice() const	// Price를 반환하는 함수
{
	return Price;
}

UTexture2D* UItemDataAsset::GetItemImage() const	//	ItemImage를 반환하는 함수
{
	return ItemImage;
}

UStaticMesh* UItemDataAsset::GetItemMesh() const // ItemMesh를 반환하는 함수
{
	return ItemMesh;
}

int32 UItemDataAsset::GetMaxStack() const	// 가지고 있을 수 있는 최대 개수를 반환하는 함수
{
	return MaxStack;
}
