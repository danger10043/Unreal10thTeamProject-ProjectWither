// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EPlayerActionState : uint8
{
	None					UMETA(DisplayName = "아무 액션도 취하고 있지 않음"),
	AttackingWithSword		UMETA(DisplayName = "검으로 공격 중"),
	Rolling					UMETA(DisplayName = "구르는 중"),
	Blocking				UMETA(DisplayName = "검으로 막는 중"),
	HitReact				UMETA(DisplayName = "적에게 피격당하는 중"),
	Dead					UMETA(DisplayName = "사망 상태")
};