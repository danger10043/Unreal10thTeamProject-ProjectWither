// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Interface/CombatComponentUserInterface.h"
#include "Interface/EquipmentComponentUserInterface.h"
#include "Interface/PlayerInterface.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class UStatComponent;
class UWeaponComponent;
class UEquipmentComponent;
class UInventoryComponent;
class UWeaponDataAsset;
class UUserWidget;
class UTestMainUIWidget;

UCLASS()
class PROJECTWITHER_API APlayerCharacter : 
	public ACharacter,
	public IStatComponentUserInterface,
	public ICombatComponentUserInterface,
	public IEquipmentComponentUserInterface,
	public IWeaponComponentUserInterface,
	public IPlayerInterface
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


	//Inventory 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category = "Player|Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintPure, Category = "Player|Inventory")
	bool IsInventoryOpen() const { return bIsInventoryOpen; }

	virtual UStatComponent* GetStatComponent_Implementation() const override;

	virtual UWeaponComponent* GetWeaponComponent_Implementation() const override;

	virtual UEquipmentComponent* GetEquipmentComponent_Implementation() const override;

	virtual UCombatComponent* GetCombatComponent_Implementation() const override;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartRun();
	void StopRun();

	void ConsumeRunStamina();

	void UpdateMovementSpeed();

	void StartRoll();

	void AttackInput();

	void StartBlockInput();
	void StopBlockInput();

	void SwapWeaponInput();

	void AddDefaultTestWeapons();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponComponent> WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/*
	*  테스트용 시작 무기.
	* PlayerCharacter Blueprint의 디폴트 창에서 지정.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Test", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponDataAsset> TestSwordData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Test", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponDataAsset> TestGunData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Test", meta = (AllowPrivateAccess = "true"))
	bool bEquipTestWeapon = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> InventoryScreenClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> InventoryScreenWidget;
	// 플레이어 UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTestMainUIWidget> TestMainUIClass;

	UPROPERTY(Transient)
	TObjectPtr<UTestMainUIWidget> TestMainUIInstance;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RollAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> BlockAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SwapWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;


	// 이동 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float WalkSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RunSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement|Run", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float RunStaminaCostPerTick = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Movement|Run", meta = (AllowPrivateAccess = "true", ClampMin = "0.01", Units = "s"))
	float RunStaminaConsumptionInterval = 0.2f;

	FTimerHandle RunStaminaTimerHandle;

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


	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Player|UI", meta = (AllowPrivateAccess = "true"))
	bool bIsInventoryOpen = false;

};
