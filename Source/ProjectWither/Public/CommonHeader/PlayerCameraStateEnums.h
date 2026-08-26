// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EPlayerCameraState : uint8
{
	None		UMETA(DisplayName = "평상시 카메라 상태"),
	Zoom			UMETA(DisplayName = "총을 장착한 상태에서 화면 줌"),
	LockOn		UMETA(DisplayName = "에임이 특정 적을 따라가도록 설정")
};