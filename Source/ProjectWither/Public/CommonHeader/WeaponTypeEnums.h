// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Sword		UMETA(DisplayName = "검"),
	Gun			UMETA(DisplayName = "총")
};