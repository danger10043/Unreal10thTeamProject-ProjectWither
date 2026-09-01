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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTWITHER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponComponent();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeapon(UWeaponDataAsset* WeaponData);

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

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon|Gun")
	bool PerformGunFire(AActor* CurrentWeaponActor);

	virtual bool PerformGunFire_Implementation(AActor* CurrentWeaponActor);

	bool ConsumeAmmo();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun")
	bool Reload();

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun")
	int32 GetCurrentAmmo() const;

private:
	bool SaveCurrentWeaponToInventory();

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
