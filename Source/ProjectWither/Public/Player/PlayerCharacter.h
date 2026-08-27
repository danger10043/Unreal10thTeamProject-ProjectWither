// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class PROJECTWITHER_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void SetCanMove(bool bNewCanMove);

	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool CanMove() const { return bCanMove; }

	UFUNCTION(BlueprintPure, Category = "Player|Movement")
	bool IsRunning() const { return bIsRunning; }

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartRun();
	void StopRun();

	void UpdateMovementSpeed();

private:
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> PlayerCamera;

	// 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RunAction;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// Move 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float WalkSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RunSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RotationRate = 720.0f;

	// 카메라 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float CameraDistance = 350.0f;

	// 현 행동 상태
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true"))
	bool bCanMove = true;

};
