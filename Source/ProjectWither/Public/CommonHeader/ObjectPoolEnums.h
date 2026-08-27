// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/** 풀이 MaxSize에 도달했을 때 새로운 획득 요청을 처리하는 정책입니다. */
UENUM(BlueprintType)
enum class EObjectPoolPolicy : uint8
{
	DoNotSpawn		UMETA(DisplayName = "더 이상 생성하지 않음"),
	Grow			UMETA(DisplayName = "계속 생성(MaxSize 무시)"),
	ReuseOldest		UMETA(DisplayName = "가장 오래된 액터 재사용")
};
