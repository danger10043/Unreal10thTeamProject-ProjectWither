// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/AmmoDataAsset.h"

UAmmoDataAsset::UAmmoDataAsset()
{
	// 탄약 데이터 에셋은 항상 아이템 타입을 Ammo로 고정
	ItemType = EItemType::Ammo;

	MaxStack = 999;
}

EAmmoType UAmmoDataAsset::GetAmmoType() const
{
	return AmmoType;
}
