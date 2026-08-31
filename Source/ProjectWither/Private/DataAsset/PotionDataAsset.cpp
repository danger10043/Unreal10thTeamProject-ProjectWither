// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/PotionDataAsset.h"

UPotionDataAsset::UPotionDataAsset()
{
	// 포션 데이터 에셋은 항상 아이템 타입을 Potion으로 고정
	ItemType = EItemType::Potion;

	// 초기값 20개
	MaxStack = 20;
}

float UPotionDataAsset::GetHealAmount() const
{
	return HealAmount;
}
