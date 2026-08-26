// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EBossMovementPhase : uint8
{
	Flying		UMETA(DisplayName = "공중"),
	Landing			UMETA(DisplayName = "착지 중"),
	Ground		UMETA(DisplayName = "지상")
};