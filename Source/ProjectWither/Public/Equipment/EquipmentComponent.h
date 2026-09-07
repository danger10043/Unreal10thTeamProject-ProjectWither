#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommonHeader/ArmorTypeEnums.h"
#include "CommonHeader/WeaponTypeEnums.h"
#include "Item/ItemInstance.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChangedDelegate);									// 장비 장착 상태가 변경되었을 때 호출되는 델리게이트

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Event")
	FOnEquipmentChangedDelegate OnEquipmentChanged;												// 바인딩 가능한 장비 변경 이벤트 델리게이트

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItemFromInventorySlot(int32 SlotIndex);											// 지정한 인벤토리 슬롯의 장비 아이템을 장착한다.

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UnequipItem(const FItemInstance& EquipmentItem);										// 지정한 장비 아이템을 해제한다.

	UFUNCTION(BlueprintPure, Category = "Equipment|Weapon")
	FItemInstance GetEquippedSword() const;														// 현재 장착 중인 근접 무기 반환

	UFUNCTION(BlueprintPure, Category = "Equipment|Weapon")
	FItemInstance GetEquippedGun() const;														// 현재 장착 중인 원거리 무기 반환

	UFUNCTION(BlueprintPure, Category = "Equipment|Armor")
	FItemInstance GetEquippedHelmet() const;													// 현재 장착 중인 헬멧 반환

	UFUNCTION(BlueprintPure, Category = "Equipment|Armor")
	FItemInstance GetEquippedChestplate() const;												// 현재 장착 중인 상의 반환

	UFUNCTION(BlueprintPure, Category = "Equipment|Armor")
	FItemInstance GetEquippedLeggings() const;													// 현재 장착 중인 하의 반환

	UFUNCTION(BlueprintPure, Category = "Equipment|Armor")
	FItemInstance GetEquippedBoots() const;														// 현재 장착 중인 신발 반환

private:
	FItemInstance* GetWeaponEquipmentSlot(EWeaponType WeaponType);								// 무기 타입에 맞는 장비 슬롯 반환

	FItemInstance* GetArmorEquipmentSlot(EArmorType ArmorType);									// 방어구 타입에 맞는 장비 슬롯 반환

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Weapon", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedSword;																// 현재 장착 중인 근접 무기

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Weapon", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedGun;																	// 현재 장착 중인 원거리 무기

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Armor", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedHelmet;																// 현재 장착 중인 헬멧

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Armor", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedChestplate;															// 현재 장착 중인 상의

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Armor", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedLeggings;																// 현재 장착 중인 하의

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Armor", meta = (AllowPrivateAccess = "true"))
	FItemInstance EquippedBoots;																// 현재 장착 중인 신발
};
