// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1		UMETA(DisplayName = "페이즈 1"),
	Transition		UMETA(DisplayName = "변환"),
	Phase2		UMETA(DisplayName = "페이즈 2"),
	Dead			UMETA(DisplayName = "사망")
};