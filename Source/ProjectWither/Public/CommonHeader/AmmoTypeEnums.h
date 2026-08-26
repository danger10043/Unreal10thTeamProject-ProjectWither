// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Normal		UMETA(DisplayName = "일반 총 탄약"),
	Special			UMETA(DisplayName = "특수 총 탄약")
};