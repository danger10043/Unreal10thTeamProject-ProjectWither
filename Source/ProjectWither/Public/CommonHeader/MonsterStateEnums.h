// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Idle		UMETA(DisplayName = "평시 상태"),
	Patrol			UMETA(DisplayName = "순찰 및 감시 중"),
	Chase		UMETA(DisplayName = "플레이어 추적 중"),
	Attack		UMETA(DisplayName = "플레이어 공격 중"),
	Search		UMETA(DisplayName = "타겟을 놓친 후 탐색 상태"),
	Hit		UMETA(DisplayName = "피격 상태"),
	Dead		UMETA(DisplayName = "사망 상태")
};