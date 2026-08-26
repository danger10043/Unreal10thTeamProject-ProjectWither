// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EObjectPoolPolicy : uint8
{
	DoNotSpawn		UMETA(DisplayName = "더 이상 생성하지 않음"),
	Grow			UMETA(DisplayName = "계속 생성(MaxSize 무시)"),
	ReuseOldest		UMETA(DisplayName = "가장 오래된 액터 재사용")
};