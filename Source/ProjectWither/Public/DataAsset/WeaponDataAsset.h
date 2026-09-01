#pragma once

#include "CoreMinimal.h"
#include "DataAsset/ItemDataAsset.h"
#include "CommonHeader/WeaponTypeEnums.h"
#include "CommonHeader/WeaponGunTypeEnums.h"
#include "CommonHeader/AmmoTypeEnums.h"
#include "NiagaraSystem.h"
#include "WeaponDataAsset.generated.h"

UCLASS(BlueprintType)
class PROJECTWITHER_API UWeaponDataAsset : public UItemDataAsset
{
	GENERATED_BODY()

public:
	UWeaponDataAsset();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")	// 무기 타입 ( 검 , 총 ) 기본 값 : 검
	EWeaponType WeaponType = EWeaponType::Sword;

	//Sword일 때 총 관련 property를 설정하는 걸 막기 위해 비활성화 하였음.  
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon",  meta = (EditCondition = "WeaponType == EWeaponType::Gun")) // 총 세부 타입 (None, 일반, 특수) 기본값 : None
	EWeaponGunType WeaponGunType = EWeaponGunType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "WeaponType == EWeaponType::Gun"))	// 총 세부 타입에 따른 탄환의 타입 (일반, 특수) 기본값 : 일반
	EAmmoType AmmoType = EAmmoType::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "WeaponType == EWeaponType::Gun", ClampMin = "0")) // 탄환의 최대 개수 기본값 : 0
	int32 MaxAmmo = 0;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0.0"))	// 무기 공격력
	float WeaponPower = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Actor")
	TSubclassOf<AActor> WeaponActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Actor")
	FName AttachSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effect")	// 검 공격 적중시 이펙트
	TObjectPtr<UNiagaraSystem> WeaponHitEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effect") // 검 궤적 이펙트
	TObjectPtr<UNiagaraSystem> WeaponTrailEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effect") // 패링 시 이펙트
	TObjectPtr<UNiagaraSystem> WeaponParryingEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effect") // 총 발사 시 이펙트
	TObjectPtr<UNiagaraSystem> WeaponFireEffect = nullptr;

public:
	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetWeaponType() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponGunType GetWeaponGunType() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EAmmoType GetAmmoType() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMaxAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetWeaponPower() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Actor")
	TSubclassOf<AActor> GetWeaponActorClass() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Actor")
	FName GetAttachSocketName() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UNiagaraSystem* GetWeaponHitEffect() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UNiagaraSystem* GetWeaponTrailEffect() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UNiagaraSystem* GetWeaponParryingEffect() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UNiagaraSystem* GetWeaponFireEffect() const;
};
