// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class APlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();

	// F를 눌렀을 때 호출
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	// 안내 문구 표시 여부 확인용
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasInteractionTarget() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetInteractionTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 플레이어가 상호작용을 시작할 수 있는 상태인지 확인
	bool CanPlayerInteract() const;

	// 시선과 거리 조건을 만족하는 대상 찾기
	AActor* FindInteractionTarget() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerCharacter> OwnerPlayer;

	// 대상이 파괴되어도 안전하게 확인할 수 있는 약한 참조
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentTarget;

	// 카메라에서 앞으로 검사할 길이
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "0.0", Units = "cm"))
	float TraceDistance = 1000.0f;

};
