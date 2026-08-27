// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/ArmorDataAsset.h"

UArmorDataAsset::UArmorDataAsset()
{
	// 방어구 데이터 에셋은 항상 아이템 타입을 Armor로 고정
	ItemType = EItemType::Armor;

	// 방어구는 장비 아이템이므로 기본적으로 중첩되지 않는 장비 아이템으로 취급
	MaxStack = 1;
}

EArmorType UArmorDataAsset::GetArmorType() const
{
	return ArmorType;
}

float UArmorDataAsset::GetArmorDefense() const
{
	return ArmorDefense;
}
