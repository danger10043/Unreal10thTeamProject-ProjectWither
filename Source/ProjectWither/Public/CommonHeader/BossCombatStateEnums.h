// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EBossCombatState : uint8
{
	Intro				UMETA(DisplayName = "인트로"),
	Combat			UMETA(DisplayName = "전투 중"),
	PhaseTransition		UMETA(DisplayName = "페이즈 변경"),
	Stunned			UMETA(DisplayName = "경직"),
	Dead				UMETA(DisplayName = "죽음")
};