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

private:
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

	UPROPERTY(Transient)
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

};
