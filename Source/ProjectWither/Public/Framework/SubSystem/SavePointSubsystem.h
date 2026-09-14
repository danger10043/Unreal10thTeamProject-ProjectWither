// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SavePointTypes.h"
#include "SavePointSubsystem.generated.h"

class UTexture2D;

// 세이브 포인트 한 곳의 활성화 상태와 휴식 위치를 담는 내부 기록
USTRUCT()
struct FSavePointRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FText DisplayName;

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	bool bActivated = false;

	UPROPERTY()
	TObjectPtr<UTexture2D> LocationImage = nullptr;
};

/*
* 세이브 포인트(휴식처)의 발견/활성화 상태와 마지막으로 쉰 위치(부활 지점)를 관리하는 Subsystem.
* GameInstance 수명 동안(현재 플레이 세션 동안) 유지되며, 게임을 완전히 재시작하면 초기화된다.
*/
UCLASS()
class PROJECTWITHER_API USavePointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 세이브 포인트 액터가 BeginPlay에서 자신을 등록. 이미 등록된 Id면 위치/이름/이미지만 갱신하고 활성화 상태는 유지한다.
	UFUNCTION(BlueprintCallable, Category = "SavePoint")
	void RegisterSavePoint(FName SavePointId, const FText& DisplayName, const FTransform& Transform, UTexture2D* LocationImage = nullptr);

	// 지정한 세이브 포인트 하나의 UI 정보를 가져온다 (현재 위치한 세이브 포인트의 이미지를 기본값으로 보여줄 때 등에 사용)
	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool GetSavePointInfo(FName SavePointId, FSavePointInfo& OutInfo) const;

	// 세이브 포인트를 활성화하고 마지막(현재) 세이브 포인트로 지정. 등록되지 않은 Id면 실패.
	UFUNCTION(BlueprintCallable, Category = "SavePoint")
	bool ActivateSavePoint(FName SavePointId);

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool IsSavePointActivated(FName SavePointId) const;

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool HasCurrentSavePoint() const;

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	FORCEINLINE FName GetCurrentSavePointId() const { return CurrentSavePointId; }

	// 마지막으로 활성화한(현재) 세이브 포인트의 Transform. 없으면 Identity 반환.
	UFUNCTION(BlueprintPure, Category = "SavePoint")
	FTransform GetCurrentSavePointTransform() const;

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool GetSavePointTransform(FName SavePointId, FTransform& OutTransform) const;

	// UI 표시용 - 현재까지 활성화된 세이브 포인트 목록 (패스트 트래블 대상)
	UFUNCTION(BlueprintPure, Category = "SavePoint")
	TArray<FSavePointInfo> GetActivatedSavePoints() const;

private:
	UPROPERTY(Transient)
	TMap<FName, FSavePointRecord> SavePoints;

	UPROPERTY(Transient)
	FName CurrentSavePointId = NAME_None;
};
