// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LockOnWidget.generated.h"

class UImage;
class USizeBox;
class UWidgetAnimation;

UCLASS()
class PROJECTWITHER_API ULockOnWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

private:
	void ActivateLockOn(AActor* Target);
	void DeactivateLockOn();
	void UpdateScreenPosition();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> RootBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LockonImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> LockonActivate;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> LockonDeactivate;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn|UI", meta = (ClampMin = "1.0"))
	float MarkerSize = 64.0f;

	// 적의 Actor 위치를 기준으로 추가할 월드 공간 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|UI")
	FVector TargetOffset = FVector(0.0f, 0.0f, 50.0f);

private:
	TWeakObjectPtr<AActor> TrackedTarget;
	FVector LastTargetLocation = FVector::ZeroVector;

	bool bLockOnActive = false;
	bool bDeactivating = false;
};
