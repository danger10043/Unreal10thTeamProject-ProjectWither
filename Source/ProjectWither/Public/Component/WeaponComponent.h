 #pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/WeaponTypeEnums.h"
#include "Item/ItemInstance.h"
#include "WeaponComponent.generated.h"

class AActor;
class UInventoryComponent;
class UStatComponent;
class UWeaponDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponChangedDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTWITHER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponComponent();

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Event")
	FOnWeaponChangedDelegate OnWeaponChanged;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(UWeaponDataAsset* WeaponData);

	bool EquipWeaponInstance(const FItemInstance& WeaponInstance);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool SwapWeapon();

	// C++ 전용
	FItemInstance* GetCurrentWeapon();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UWeaponDataAsset* GetCurrentWeaponData() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AActor* GetWeaponActor() const { return IsValid(WeaponActor) ? WeaponActor.Get() : nullptr; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetWeaponType() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsSwordEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsGunEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun")
	bool FireGun();

	bool ConsumeAmmo();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun")
	bool Reload();

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun")
	int32 GetCurrentAmmo() const;

	// 인벤토리 탄약을 소모하지 않고 장착 중인 총의 탄창을 최대치로 채운다 (세이브 포인트 휴식 등에서 사용)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun")
	bool RefillCurrentWeaponAmmo();

private:
	bool SaveCurrentWeaponToInventory();
	void SyncCurrentWeaponToEquipment();

	AActor* SpawnWeaponActor(const UWeaponDataAsset* WeaponData) const;
	
	void DestroyWeaponActor();

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	FItemInstance CurrentWeapon;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	int32 CurrentWeaponSlot = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> WeaponActor = nullptr;
};
