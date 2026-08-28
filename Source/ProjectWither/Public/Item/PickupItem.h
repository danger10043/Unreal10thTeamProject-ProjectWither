// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupItem.generated.h"

class UItemDataAsset;
class USphereComponent;
class UNiagaraComponent;
class UMeshComponent;
class UStaticMeshComponent;

UCLASS()
class PROJECTWITHER_API APickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

	void InitializePickup(UItemDataAsset* InItemData, int32 InQuantity);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 오버랩 델리게이트에 바인딩 할 함수
	UFUNCTION()
	void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	// 오버랩 됬을 때 대상에게 실제 작업을 처리하는 함수
	virtual void OnPickup(AActor* InTarget);

	virtual void OnUpdatePickupEffect();
	virtual void OnFinishPickupEffect();

	virtual void OnUpdateUpdownSpin(float InDeltaTime);

	virtual UMeshComponent* GetMesh() const;
private:
	bool IsCurveAssetReady() const;
	bool IsPickupEffectAssetReady() const;

protected:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UItemDataAsset> ItemData = nullptr; // 픽업시 획득할 아이템 데이터 에셋

	UPROPERTY(VisibleInstanceOnly)
	int32 Quantity = 0;		// 픽업시 획득할 아이템의 개수

	// 메시의 기본 위치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Data")
	FVector MeshBaseLocation = FVector(0, 0, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Data")
	float PickupDelayTime = 1.0f; // 스폰 직후에 아이템이 안먹어지는 시간

	// 맵에 있을 때 위아래로 왕복하는 모습용 커브
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Spawn")
	TObjectPtr<UCurveFloat> UpDownCurve;

	// 맵에 있을 때 회전하는 모습용 커브
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Spawn")
	TObjectPtr<UCurveFloat> SpinCurve;

	// 위아래로 왕복하는데 걸리는 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Spawn", meta = (ClampMin = "0.001"))
	float UpDownDuration = 2.0f;

	// 위아래로 움직이는 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Spawn")
	float UpDownHeight = 100.0f;

	// 아이템을 줍는 연출의 진행 상황용 커브
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Pickup")
	TObjectPtr<UCurveFloat> PickupAlpha;

	// 아이템을 줍는 연출 중 위 아래로 움직임을 위한 커브
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Pickup")
	TObjectPtr<UCurveFloat> PickupHeight;

	// 아이템을 줍는 연출 중 크기 변경을 위한 커브
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Pickup")
	TObjectPtr<UCurveFloat> PickupScale;

	// 아이템을 줍는 연출의 전체 진행 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Pickup")
	float PickupEffectDuration = 0.5f;

	// PickupHeight로 인해 올라가는 높이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Pickup")
	float PickupEffecHeight = 50.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereCollision = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

private:
	// 스폰 연출용 누적 시간
	float ElapsedTime = 0.0f;

	// 스폰용 연출 On/Off
	bool bIdle = true;

	// 아이템을 줍는 연출용 타이머 핸들
	FTimerHandle PickupEffectTimerHandle;

	// 아이템을 줍는 대상
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	// 아이템을 줍는 연출이 진행된 시간
	float PickupElapsedTime = 0.0f;

	// 아이템을 줍는 연출용 타이머의 실행 간격
	const float TimerInterval = 0.02f;

	// 아이템을 줍는 연출용 시작 위치
	FVector PickupStartLocation;
};
