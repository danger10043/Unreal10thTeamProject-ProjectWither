// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "CommonHeader/MonsterDespawnEnums.h"
#include "TimerManager.h"
#include "MonsterComponent.generated.h"

class UStatComponent;
class UItemDataAsset;
class UAnimMontage;
class UDataTable;
class APickupItem;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterAttackFinished, bool, bInterrupted);

UCLASS(ClassGroup=(Monster), meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UMonsterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Shared gameplay state for both Character and Pawn monsters.
	UMonsterComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Both monster actor types forward their damage here.
    float ApplyMonsterDamage(float Damage);

    UFUNCTION(BlueprintPure, Category = "Monster")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    int32 GetMonsterId() const { return MonsterId; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    EMonsterState GetMonsterState() const { return MonsterState; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    FVector GetSpawnLocation() const { return SpawnLocation; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    float GetAllowRange() const { return AllowRange; }

    UPROPERTY(BlueprintAssignable, Category = "Monster")
    FOnMonsterDied OnMonsterDied;

	UPROPERTY(BlueprintAssignable, Category = "Monster|Combat")
	FOnMonsterAttackFinished OnMonsterAttackFinished;

	UFUNCTION(BlueprintCallable)
	void SetMonsterState(EMonsterState NewState); // 몬스터 상태 변경

	UFUNCTION(BlueprintCallable)
	void SetTarget(AActor* NewTarget); // 현재 타겟 설정

	UFUNCTION(BlueprintCallable)
	void ClearTarget(); // 현재 타겟 제거

	UFUNCTION(BlueprintCallable)
	AActor* GetTargetActor() const; // 현재 타겟 반환

	UFUNCTION(BlueprintCallable)
	float GetDistanceToTarget() const; // 현재 타겟까지 거리 반환

	UFUNCTION(BlueprintCallable)
	void CalculateDrops();		// 드랍 여부 및 개수 결정

	UFUNCTION(BlueprintCallable)
	void DropItems();			// 계산된 아이템을 월드에 생성

	UFUNCTION(BlueprintCallable)
	bool IsInAttackRange();		// 유효한 타겟이 공격 거리 안에 있는지

	UFUNCTION(BlueprintCallable)
	bool CanAttack();			// 생존·타겟·거리·쿨타임을 검사

	UFUNCTION(BlueprintCallable)
	bool Attack();				// 공격

	UFUNCTION(BlueprintCallable)
	void FinishAttack();		// 공격 종료 상태 처리

	UFUNCTION(BlueprintCallable)
	void ResetAttackCooldown();	// 쿨타임 종료 처리

	UFUNCTION(BlueprintCallable)
	void ApplyAttackDamage(AActor* HitTarget, float AttackMultiplier = 1.0f);	// 실제 타격 판정, 피해 적용

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void RegisterAttackHitbox(FName HitboxName, UPrimitiveComponent* Hitbox);	// 공격 히트 박스 등록

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void BeginAttackHitWindow(FName HitboxName);	// 공격 시작, 히트 킴

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void EndAttackHitWindow(FName HitboxName);		// 공격 종료, 히트 끔

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void CancelAttack();							// 공격 취소

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void PlayHitReaction();							// 피격 애니메이션 재생

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void HandleParried();							// 패링 처리

private:
	UFUNCTION()
	void HandleDeath();

	FName SelectAttackSection() const;	// 공격 애니메이션 섹션 랜덤 선택 함수

	void DisableAllAttackHitboxes();	// 모든 공격 히트 박스 비활성화

	UFUNCTION()	// 공격 히트박스 오버랩
	void OnAttackHitboxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ProcessAttackOverlap(AActor* OtherActor); // 실제 오버랩 구현

	// 공격 몽타주 종료 후
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 반응 몽타주 재생
	void PlayReactionMontage(UAnimMontage* Montage);

	// 피격 몽타주 종료 후
	void OnReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 사망 몽타주 재생
	void PlayDeathMontage();

	// 사망 몽타주 종료 후
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);	

	// 사망 후처리
	void FinishDeath();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	int32 MonsterId = 0; // 몬스터 고유 ID

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	EMonsterState MonsterState = EMonsterState::Idle; // 현재 몬스터 상태

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base")
	TObjectPtr<UStatComponent> StatComponent = nullptr; // 스탯 컴포넌트

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	FVector SpawnLocation = FVector(0, 0, 0); // 최초 생성 위치

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Death")
	EMonsterDespawnPolicy DespawnPolicy = EMonsterDespawnPolicy::Destroy;	// 몬스터 디스폰 정책

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Death",
		meta = (ClampMin = "0.0", Units = "s"))
	float DespawnDelay = 5.0f;	// 디스폰 딜레이 시간

	// Drop --------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TObjectPtr<UDataTable> ItemDropTable = nullptr;	// 몬스터 드랍 테이블

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<APickupItem> ItemPickupClass;	// 아이템 픽업 클래스

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropRange = 100.0f;		// 드랍 아이템 랜덤 범위 (X, Y)

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropHeight = 20.0f;		// 드랍 아이템 랜덤 높이 (Z)
	// -------------------------------------------------------------------------

	// Combat ------------------------------------------------------------------
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	TObjectPtr<AActor> TargetActor = nullptr; // 현재 추적 또는 공격 대상

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	bool bIsDead = false;		// 사망 여부

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base")
	bool bCanAttack = true;	// 공격 가능 여부

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float AllowRange = 100.0f;	// 플레이어에게 최대한 접근할 수 있는 거리

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float AttackRange = 400.0f;	// 공격 범위

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base")
	float AttackCooldown = 1.0f;	// 공격 간격

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (ClampMin = "1.0"))
	float DefenseScalingConstant = 100.0f;
	// -------------------------------------------------------------------------

protected:
	// Montage -----------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;	// 공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	FString AttackSectionPrefix = TEXT("Attack_");

	UPROPERTY(Transient)
	FName LastAttackSection = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> HitReactMontage = nullptr;	// 피격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> ParriedMontage = nullptr;	// 패링 반응 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;	// 사망 몽타주
	// -------------------------------------------------------------------------

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<UItemDataAsset>, int32> DropItem;	// 계산 후 확정된 드랍 아이템들

	// 공격 콜리전 모음
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UPrimitiveComponent>> AttackHitboxes;

	// 현재 활성화한 공격 콜리전
	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> ActiveAttackHitbox = nullptr;

	UPROPERTY(Transient)
	FName ActiveHitboxName = NAME_None;			// 현재 활성화된 공격 콜리전 이름

	TSet<TWeakObjectPtr<AActor>> HitActors; 	// 같은 타격 구간에서 중복 처리 방지

	FTimerHandle AttackCooldownTimerHandle;		// 공격 쿨타임 타이머핸들

	FTimerHandle DespawnTimerHandle;			// 디스폰 타이머핸들
};
