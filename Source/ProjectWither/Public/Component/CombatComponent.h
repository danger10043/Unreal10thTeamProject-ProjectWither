// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "CommonHeader/PlayerActionStateEnums.h"
#include "CombatComponent.generated.h"

// WeaponComponent의 실제 무기 Enum이 정해지기 전 사용하는 연결용 타입. 추후 삭제 예정
UENUM(BlueprintType)
enum class ECombatWeaponType : uint8
{
	None,
	Sword,
	Gun
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnPlayerActionStateChangedDelegate,
	EPlayerActionState, PreviousState,
	EPlayerActionState, NewState
);

class APlayerCharacter;
class UStatComponent;
class UWeaponComponent;
class UAnimMontage;
class UCurveFloat;
class UCapsuleComponent;
class UPrimitiveComponent;

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Attack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SwordAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat|Sword")
	void BeginSwordDamageWindow();
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Sword")
	void EndSwordDamageWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void GunAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Roll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	float ReceiveHit(float DamageAmount, AActor* DamageCauser, AController* EventInstigator );

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Die();

	// 현재 동작의 종료 또는 중단 처리에서 호출
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FinishAction(EPlayerActionState ExpectedState);

	UFUNCTION(BlueprintPure, Category = "Combat")
	FORCEINLINE EPlayerActionState GetActionState() const { return ActionState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanRoll() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanBlock() const;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Event")
	FOnPlayerActionStateChangedDelegate OnActionStateChangedEvent;

protected:
	// WeaponComponent의 실제 API에 연결할 함수
	UFUNCTION(BlueprintNativeEvent, Category = "Combat|Weapon")
	ECombatWeaponType ResolveWeaponType() const;

	virtual ECombatWeaponType ResolveWeaponType_Implementation() const;

	// 검 공격/총 공격 애니메이션과 무기 동작을 연결
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnAttackStarted(ECombatWeaponType WeaponType);

	// 구르기 애니메이션과 이동 처리를 연결
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnRollStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnBlockSucceeded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnParrySucceeded();

	// 같은 HitReact 상태에서 다시 맞아도 호출
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnHitReceived();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Event")
	void OnActionStateChanged(
		EPlayerActionState PreviousState,
		EPlayerActionState NewState
	);

private:
	bool IsOwnerAlive() const;

	bool TrySpendStamina(float Cost);

	void StartAttack(ECombatWeaponType RequiredWeapon, EPlayerActionState AttackState, float StaminaCost);

	void OpenParryWindow();

	void CloseParryWindow();

	void EndParryCooldown();

	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleSwordCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UCapsuleComponent* FindSwordCollision() const;

	float CalculateSwordDamage() const;

	// 외부에서 행동 검사 없이 상태를 바꾸지 못하도록 제한
	void SetActionState(EPlayerActionState State);

	void ConsumeBlockStamina();

	void StartHitReaction();

	void OnHitReactMontageEnded(
		UAnimMontage* Montage,
		bool bInterrupted
	);

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerCharacter> OwnerPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponComponent> WeaponComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EPlayerActionState ActionState = EPlayerActionState::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Roll", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> RollMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Roll", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> RollSpeedCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SwordAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Death", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DeathMontage;

	/*
	* 하나의 SwordAttackAnimNotifyState 구간에서 이미 피해를 받은 적
	* NotifyBegin에서 초기화되고 NotifyEnd에서 다시 비워짐.
	*/
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> SwordHitActors;

	// 공격 도중 무기가 교체되어도 원래 활성화했던 캡슐을 비활성화 하기 위해 저장.
	UPROPERTY(Transient)
	TObjectPtr<UCapsuleComponent> ActiveSwordCollision = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Roll", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float RollDistance = 500.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float RollStaminaCost = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float BlockStaminaCost = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float SwordAttackStaminaCost = 15.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float GunAttackStaminaCost = 5.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.0", Units = "s"))
	float ParryWindow = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry", meta = (ClampMin = "0.0", Units = "s"))
	float ParryCooldown = 1.0f;

	// 가드 유지 스태미나 소모 주기
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.01", Units = "s"))
	float BlockHoldStaminaInterval = 0.2f;
	
	// 가드를 유지하면서 한 번에 소모하는 스태미나
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina", meta = (ClampMin = "0.0"))
	float BlockHoldStaminaCost = 1.0f;

	bool bParryWindowOpen = false;

	bool bParryOnCooldown = false;

	FTimerHandle ParryTimerHandle;

	FTimerHandle ParryCooldownTimerHandle;

	FTimerHandle BlockStaminaTimerHandle; 
};
