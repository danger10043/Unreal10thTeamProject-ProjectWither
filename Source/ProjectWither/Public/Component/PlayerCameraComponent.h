#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/PlayerCameraStateEnums.h"
#include "PlayerCameraComponent.generated.h"

class APlayerCharacter;
class UCameraComponent;
class USpringArmComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTWITHER_API UPlayerCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerCameraComponent();

	// 플레이어가 사용하는 실제 카메라와 암 연결
	void InitializeCamera(
		UCameraComponent* InCameraComponent,
		USpringArmComponent* InSpringComponent
	);

	// 기존 LookAction의 입력값 전달받기
	void HandleLookInput(const FVector2D& Input);

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void StartZoom();

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void StopZoom();

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void ChangeCameraState(EPlayerCameraState NewState);

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	AActor* FindLockOnTarget();

	UFUNCTION(BlueprintCallable, Category = "Player|Camera")
	void ClearLockOn();

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	void UpdateCameraPlacement(float DeltaTime);
	void UpdateCameraFOV(float DeltaTime);
	bool CanZoom() const;

	bool IsValidLockOnTarget(AActor* Target) const;

	UPROPERTY(Transient)
	TObjectPtr<APlayerCharacter> OwnerPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CameraComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> SpringArmComponent = nullptr;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Camera",
		meta = (AllowPrivateAccess = "true")
	)
	EPlayerCameraState CameraState = EPlayerCameraState::None;

	UPROPERTY(
		Transient,
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Camera",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<AActor> LockonTarget = nullptr;
	
	// 초기화 시 실제 카메라의 FOV를 기본값으로 저장
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Player|Camera",
		meta = (AllowPrivateAccess = "true")
	)
	float NormalFOV = 90.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "5.0",
			ClampMax = "170.0"
			)
	)
	float ZoomFOV = 60.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0",
			Units = "cm"
			)
	)
	float LockOnRange = 2000.0f;

	// 카메라 위치 초기화시 저장하는 최초 SpringArm 배치
	UPROPERTY(Transient)
	FVector NormalSocketOffset = FVector::ZeroVector;

	UPROPERTY(Transient)
	float NormalArmLength = 350.0f;

	// 기본 배치에서 추가로 이동할 거리
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera|Placement",
		meta = (
			AllowPrivateAccess = "true",
			Units = "cm"
		)
	)
	FVector RangedSocketOffset = FVector(0.0f, 70.0f, 50.0f);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera|Placement",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0",
			Units = "cm"
		)
	)
	float RangedArmLength = 250.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera|Placement",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.1"
		)
	)
	float CameraPlacementInterpSpeed = 8.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Player|Camera|Zoom",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.1"
		)
	)
	float ZoomInterpSpeed = 10.0f;
};
