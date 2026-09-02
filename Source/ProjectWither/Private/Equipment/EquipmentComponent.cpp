#include "Equipment/EquipmentComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/WeaponComponent.h"
#include "CommonHeader/ItemTypeEnums.h"
#include "DataAsset/ArmorDataAsset.h"
#include "DataAsset/ItemDataAsset.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/Actor.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UEquipmentComponent::EquipItemFromInventorySlot(int32 SlotIndex)
{
	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))																		// 장비 컴포넌트를 소유한 액터가 없으면 장착할 수 없다.
	{
		return false;
	}

	UInventoryComponent* InventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();

	if (!IsValid(InventoryComponent))																// 장착할 아이템을 가져올 인벤토리 컴포넌트가 없으면 실패 처리
	{
		return false;
	}

	FItemInstance InventoryItem;

	if (!InventoryComponent->GetItemAtSlot(SlotIndex, InventoryItem))								// 지정한 인벤토리 슬롯의 아이템 정보를 가져온다.
	{
		return false;
	}

	if (InventoryItem.ItemData == nullptr || InventoryItem.Quantity <= 0)							// 빈 슬롯이거나 수량이 없는 아이템은 장착할 수 없다.
	{
		return false;
	}

	FItemInstance* TargetEquipmentSlot = nullptr;

	switch (InventoryItem.ItemData->GetItemType())
	{
	case EItemType::Weapon:
	{
		UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(InventoryItem.ItemData.Get());

		if (!IsValid(WeaponData))																	// 무기 아이템 타입이지만 무기 데이터 에셋이 아니면 실패 처리
		{
			return false;
		}

		switch (WeaponData->GetWeaponType())
		{
		case EWeaponType::Sword:
		{
			UWeaponComponent* WeaponComponent = OwnerActor->FindComponentByClass<UWeaponComponent>();

			if (!IsValid(WeaponComponent) || !WeaponComponent->EquipWeapon(WeaponData))				// 무기 실제 장착은 기존 WeaponComponent에 맡긴다.
			{
				return false;
			}

			EquippedSword = InventoryItem;															// WeaponComponent 구조 유지를 위해 무기는 인벤토리 슬롯에서 제거하지 않는다.
			OnEquipmentChanged.Broadcast();															// 장비 슬롯 UI가 갱신될 수 있도록 장착 변경을 알린다.
			return true;
		}
		case EWeaponType::Gun:
		{
			UWeaponComponent* WeaponComponent = OwnerActor->FindComponentByClass<UWeaponComponent>();

			if (!IsValid(WeaponComponent) || !WeaponComponent->EquipWeapon(WeaponData))				// 무기 실제 장착은 기존 WeaponComponent에 맡긴다.
			{
				return false;
			}

			EquippedGun = InventoryItem;															// WeaponComponent 구조 유지를 위해 무기는 인벤토리 슬롯에서 제거하지 않는다.
			OnEquipmentChanged.Broadcast();															// 장비 슬롯 UI가 갱신될 수 있도록 장착 변경을 알린다.
			return true;
		}
		default:
			return false;
		}
	}
	case EItemType::Armor:
	{
		UArmorDataAsset* ArmorData = Cast<UArmorDataAsset>(InventoryItem.ItemData.Get());

		if (!IsValid(ArmorData))																	// 방어구 아이템 타입이지만 방어구 데이터 에셋이 아니면 실패 처리
		{
			return false;
		}

		switch (ArmorData->GetArmorType())
		{
		case EArmorType::Helmet:
			TargetEquipmentSlot = &EquippedHelmet;
			break;
		case EArmorType::Chestplate:
			TargetEquipmentSlot = &EquippedChestplate;
			break;
		case EArmorType::Leggings:
			TargetEquipmentSlot = &EquippedLeggings;
			break;
		case EArmorType::Boots:
			TargetEquipmentSlot = &EquippedBoots;
			break;
		default:
			return false;
		}

		break;
	}
	default:
		return false;
	}

	if (TargetEquipmentSlot == nullptr)
	{
		return false;
	}

	const FItemInstance PreviousEquippedItem = *TargetEquipmentSlot;
	*TargetEquipmentSlot = InventoryItem;															// 인벤토리 아이템을 알맞은 장비 슬롯에 장착한다.

	if (!InventoryComponent->SetItemAtSlot(SlotIndex, PreviousEquippedItem))						// 기존 장비가 있으면 인벤토리로 되돌리고, 없으면 인벤토리 슬롯을 비운다.
	{
		*TargetEquipmentSlot = PreviousEquippedItem;
		return false;
	}

	OnEquipmentChanged.Broadcast();																	// 장비 슬롯 UI가 갱신될 수 있도록 장착 변경을 알린다.
	return true;
}

FItemInstance UEquipmentComponent::GetEquippedSword() const
{
	return EquippedSword;
}

FItemInstance UEquipmentComponent::GetEquippedGun() const
{
	return EquippedGun;
}

FItemInstance UEquipmentComponent::GetEquippedHelmet() const
{
	return EquippedHelmet;
}

FItemInstance UEquipmentComponent::GetEquippedChestplate() const
{
	return EquippedChestplate;
}

FItemInstance UEquipmentComponent::GetEquippedLeggings() const
{
	return EquippedLeggings;
}

FItemInstance UEquipmentComponent::GetEquippedBoots() const
{
	return EquippedBoots;
}
