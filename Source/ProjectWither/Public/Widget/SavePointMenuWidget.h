// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/SavePointTypes.h"
#include "SavePointMenuWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
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

	// 빠른 이동 목록의 항목에 마우스를 올렸을 때 호출. 해당 장소의 이미지를 미리보기 칸에 표시한다.
	UFUNCTION(BlueprintCallable, Category = "UI|SavePoint")
	void SetPreviewImage(UTexture2D* Image);

	// 빠른 이동 목록의 항목에서 마우스가 벗어났을 때 호출. 미리보기를 현재 위치한 세이브 포인트의 이미지로 되돌린다.
	UFUNCTION(BlueprintCallable, Category = "UI|SavePoint")
	void RestorePreviewImage();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

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

	// 현재 위치/빠른 이동 목록 항목의 장소 이미지를 보여주는 미리보기 칸
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PreviewImage;

	FName CurrentSavePointId = NAME_None;

	// 마우스가 목록에서 벗어났을 때 되돌아갈 기본 이미지 (현재 위치한 세이브 포인트의 이미지)
	UPROPERTY()
	TObjectPtr<UTexture2D> DefaultPreviewImage = nullptr;
};
