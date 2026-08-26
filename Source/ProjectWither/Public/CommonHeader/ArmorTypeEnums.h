// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Helmet		UMETA(DisplayName = "투구"),
	Chestplate			UMETA(DisplayName = "갑옷"),
	Leggings		UMETA(DisplayName = "바지"),
	Boots		UMETA(DisplayName = "신발")
};