// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/SavePointTypes.h"
#include "SavePointMenuWidget.generated.h"

class UButton;
class UTextBlock;
class USavePointSubsystem;

/*
* 세이브 포인트 상호작용 시 열리는 메뉴.
* 휴식은 세이브 포인트 상호작용 시점에 이미 처리되므로, 이 위젯은 결과 안내와
* 다른 활성화된 세이브 포인트로의 순간이동(패스트 트래블)을 담당한다.
* 목록 UI 구성(버튼 생성 등)은 GetTravelDestinations의 결과를 이용해 블루프린트에서 담당한다.
*/
UCLASS()
class PROJECTWITHER_API USavePointMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 세이브 포인트 상호작용 직후 호출. 현재 위치한 세이브 포인트를 전달받아 화면을 갱신한다.
	UFUNCTION(BlueprintCallable, Category = "UI|SavePoint")
	void OpenAt(FName InCurrentSavePointId);

	// 다른 활성화된 세이브 포인트 목록 (현재 위치 제외)
	UFUNCTION(BlueprintPure, Category = "UI|SavePoint")
	TArray<FSavePointInfo> GetTravelDestinations() const;

	// 지정한 세이브 포인트로 순간이동. 성공하면 메뉴를 닫는다.
	UFUNCTION(BlueprintCallable, Category = "UI|SavePoint")
	bool TravelTo(FName TargetSavePointId);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 이 위젯은 세이브 포인트 액터가 재사용하는 캐싱된 인스턴스라 Event Construct는 처음 한 번만 실행된다.
	// 상호작용마다(=OpenAt 호출마다) 순간이동 목록을 새로 그려야 하므로, 블루프린트에서는
	// Event Construct 대신 이 이벤트에서 GetTravelDestinations로 목록 UI를 갱신할 것.
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|SavePoint")
	void OnMenuOpened();

private:
	void BindButtons();
	void UnbindButtons();
	void CloseAndRestoreInput();

	USavePointSubsystem* GetSavePointSubsystem() const;

	UFUNCTION()
	void HandleCloseClicked();

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RestedAtText;

	FName CurrentSavePointId = NAME_None;
};
