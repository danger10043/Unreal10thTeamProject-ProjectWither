// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon		UMETA(DisplayName = "무기"),
	Potion		UMETA(DisplayName = "물약"),
	Ammo		UMETA(DisplayName = "탄약"),
	Armor		UMETA(DisplayName = "방어구"),
	Material	UMETA(DisplayName = "강화 및 제작 재료"),
	Misc		UMETA(DisplayName = "기타")
};
