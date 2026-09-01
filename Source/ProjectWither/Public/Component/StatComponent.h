#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "StatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnHealthChangedDelegate,
	float, CurrentHealth,
	float, MaxHealth,
	float, ChangedAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnStaminaChangedDelegate,
	float, CurrentStamina,
	float, MaxStamina,
	float, ChangedAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthZeropDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTWITHER_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ResetStat();

	/*
	*	Amount 만큼 체력을 회복합니다.
	*	@return 실제로 회복된 체력
	*/
	UFUNCTION(BlueprintCallable, Category = "Stat|Health")
	float RecoverHealth(float Amount);

	/*
	*	방어력 계산이 끝난 최종 피해를 적용합니다.
	*	@return 실제로 감소한 체력
	*/
	UFUNCTION(BlueprintCallable, Category = "Stat|Health")
	float ApplyDamage(float DamageAmount);

	/*
	*	Amount 만큼 스태미나를 회복합니다.
	*	@return 실제로 회복된 스태미나
	*/
	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	float RecoverStamina(float Amount);

	/*
	*	스태미나가 충분한 경우에만 Amount만큼 사용합니다.
	*	@return 실제로 사용된 스태미나, 부족하면 0
	*/
	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	float UseStamina(float Amount);

	UFUNCTION(BlueprintPure, Category = "Stat|Stamina")
	FORCEINLINE bool HasEnoughStamina(float Amount) const { return Amount <= CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	FORCEINLINE bool IsHealthZero() const { return CurrentHealth <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Stat|Stamina")
	FORCEINLINE float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Stat|Stamina")
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Stat|Combat")
	FORCEINLINE float GetMinAttackPower() const { return MinAttackPower; }

	UFUNCTION(BlueprintPure, Category = "Stat|Combat")
	FORCEINLINE float GetMaxAttackPower() const { return MaxAttackPower; }

	UFUNCTION(BlueprintPure, Category = "Stat|Combat")
	FORCEINLINE float GetDefensePower() const { return DefensePower; }

public:
	/*
	* 체력 변동 시 호출되는 Delegate
	* 이 Event는 3개의 float 를 전달함 - {변동 후 현재 체력, 최대 체력, 실제 변동된 체력의 양}
	*/
	UPROPERTY(BlueprintAssignable, Category = "Stat|Event")
	FOnHealthChangedDelegate OnHealthChanged;

	/*
	* 스태미나 변동 시 호출되는 Delegate
	* 이 Event는 3개의 float 를 전달함 - {변동 후 현재 스태미나, 최대 체력, 실제 변동된 스태미너의 양}
	*/
	UPROPERTY(BlueprintAssignable, Category = "Stat|Event")
	FOnStaminaChangedDelegate OnStaminaChanged;

	// 체력이 0이 될 시 호출되는 Delegate
	UPROPERTY(BlueprintAssignable, Category = "Stat|Event")
	FOnHealthZeropDelegate OnHealthZero;

private:
	void RestartStaminaRecoveryDelay();

	void StartStaminaRecovery();

	void RecoverStaminaTick();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat|Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Health", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat|Stamina", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina|Recovery", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "s"))
	float StaminaRecoveryDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina|Recovery", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "s"))
	float StaminaRecoveryInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina|Recovery", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StaminaRecoveryAmountPerTick = 2.0f;

	FTimerHandle StaminaRecoveryDelayTimerHandle;

	FTimerHandle StaminaRecoveryTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinAttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxAttackPower = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Combat", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DefensePower = 0.0f;

};
