// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMonsterDespawnPolicy : uint8
{
	ReturnToPool UMETA(DisplayName = "오브젝트 풀로 반환"),
	Destroy      UMETA(DisplayName = "Actor 제거"),
	KeepCorpse   UMETA(DisplayName = "시체 유지")
};