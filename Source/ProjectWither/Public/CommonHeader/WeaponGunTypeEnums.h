// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EWeaponGunType : uint8
{
	None		UMETA(DisplayName = "총이 아닌 무기"),
	Normal			UMETA(DisplayName = "일반 총"),
	Special		UMETA(DisplayName = "특수 총")
};