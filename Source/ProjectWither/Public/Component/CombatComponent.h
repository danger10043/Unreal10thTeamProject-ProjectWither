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

class APlayerCharacter;
class UStatComponent;
class UWeaponComponent;
class UAnimMontage;

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

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void GunAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Roll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveHit();

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

	void StartAttack(
		ECombatWeaponType RequiredWeapon,
		EPlayerActionState AttackState,
		float StaminaCost
	);

	void OpenParryWindow();

	void CloseParryWindow();

	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 외부에서 행동 검사 없이 상태를 바꾸지 못하도록 제한
	void SetActionState(EPlayerActionState State);

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerCharacter> OwnerPlayer;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent;

	UPROPERTY(
		Transient,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UWeaponComponent> WeaponComponent;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (AllowPrivateAccess = "true")
	)
	EPlayerActionState ActionState = EPlayerActionState::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Roll", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> RollMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina",
		meta = (ClampMin = "0.0"))
	float RollStaminaCost = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina",
		meta = (ClampMin = "0.0"))
	float BlockStaminaCost = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina",
		meta = (ClampMin = "0.0"))
	float SwordAttackStaminaCost = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina",
		meta = (ClampMin = "0.0"))
	float GunAttackStaminaCost = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Parry",
		meta = (ClampMin = "0.0", Units = "s"))
	float ParryWindow = 0.2f;

	bool bParryWindowOpen = false;

	FTimerHandle ParryTimerHandle;
};
