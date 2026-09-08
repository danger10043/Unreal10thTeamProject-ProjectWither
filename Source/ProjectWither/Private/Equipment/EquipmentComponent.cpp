#include "Equipment/EquipmentComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/WeaponComponent.h"
#include "CommonHeader/ItemTypeEnums.h"
#include "DataAsset/ArmorDataAsset.h"
#include "DataAsset/ItemDataAsset.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/Actor.h"

namespace
{
	float GetArmorDefensePowerBonusFromItem(const FItemInstance& EquipmentItem)
	{
		if (!IsValid(EquipmentItem.ItemData) || EquipmentItem.Quantity <= 0)
		{
			return 0.0f;
		}

		const UArmorDataAsset* ArmorData =
			Cast<UArmorDataAsset>(EquipmentItem.ItemData.Get());

		return IsValid(ArmorData) ? FMath::Max(0.0f, ArmorData->GetArmorDefense()) : 0.0f;
	}
}

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
	UWeaponComponent* WeaponComponent = nullptr;
	bool bShouldActivateWeapon = false;

	switch (InventoryItem.ItemData->GetItemType())
	{
	case EItemType::Weapon:
	{
		UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(InventoryItem.ItemData.Get());

		if (!IsValid(WeaponData))																	// 무기 아이템 타입이지만 무기 데이터 에셋이 아니면 실패 처리
		{
			return false;
		}

		WeaponComponent = OwnerActor->FindComponentByClass<UWeaponComponent>();

		if (!IsValid(WeaponComponent))																// 무기를 실제 손에 장착하려면 WeaponComponent가 필요하다.
		{
			return false;
		}

		if (FItemInstance* CurrentWeapon = WeaponComponent->GetCurrentWeapon())
		{
			UpdateEquippedWeaponState(*CurrentWeapon);
		}

		TargetEquipmentSlot = GetWeaponEquipmentSlot(WeaponData->GetWeaponType());

		if (TargetEquipmentSlot == nullptr)
		{
			return false;
		}

		bShouldActivateWeapon = true;
		break;
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

	if (bShouldActivateWeapon &&
		(!IsValid(WeaponComponent) || !WeaponComponent->EquipWeaponInstance(*TargetEquipmentSlot)))	// 장비 슬롯에 들어간 무기를 실제 손에 장착한다.
	{
		InventoryComponent->SetItemAtSlot(SlotIndex, InventoryItem);
		*TargetEquipmentSlot = PreviousEquippedItem;
		OnEquipmentChanged.Broadcast();
		return false;
	}

	OnEquipmentChanged.Broadcast();																	// 장비 슬롯 UI가 갱신될 수 있도록 장착 변경을 알린다.
	return true;
}

bool UEquipmentComponent::UnequipItem(const FItemInstance& EquipmentItem)
{
	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))																		// 장비 컴포넌트를 소유한 액터가 없으면 해제할 수 없다.
	{
		return false;
	}

	if (!IsValid(EquipmentItem.ItemData) || EquipmentItem.Quantity <= 0)							// 빈 장비 슬롯은 해제하지 않는다.
	{
		return false;
	}

	switch (EquipmentItem.ItemData->GetItemType())
	{
	case EItemType::Weapon:
	{
		UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(EquipmentItem.ItemData.Get());

		if (!IsValid(WeaponData))																	// 아이템 타입은 Weapon이지만 무기 데이터가 아니면 실패 처리
		{
			return false;
		}

		FItemInstance* TargetEquipmentSlot = GetWeaponEquipmentSlot(WeaponData->GetWeaponType());

		if (TargetEquipmentSlot == nullptr ||
			!IsValid(TargetEquipmentSlot->ItemData) ||
			TargetEquipmentSlot->Quantity <= 0 ||
			TargetEquipmentSlot->ItemData.Get() != EquipmentItem.ItemData.Get())
		{
			return false;
		}

		UWeaponComponent* WeaponComponent = OwnerActor->FindComponentByClass<UWeaponComponent>();

		if (!IsValid(WeaponComponent))																// 실제 손에 든 무기 상태도 함께 정리해야 하므로 WeaponComponent가 필요하다.
		{
			return false;
		}

		if (FItemInstance* CurrentWeapon = WeaponComponent->GetCurrentWeapon())
		{
			UpdateEquippedWeaponState(*CurrentWeapon);
		}

		UInventoryComponent* InventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();

		if (!IsValid(InventoryComponent) ||
			!InventoryComponent->AddItemInstanceToEmptySlot(*TargetEquipmentSlot))					// 빈 인벤토리 슬롯이 없으면 장비 슬롯을 비우지 않는다.
		{
			return false;
		}

		if (WeaponComponent->GetWeaponType() == WeaponData->GetWeaponType())
		{
			WeaponComponent->UnequipWeapon();														// 현재 사용 중인 무기라면 실제 무기 액터도 해제한다.
		}

		*TargetEquipmentSlot = FItemInstance();
		OnEquipmentChanged.Broadcast();
		return true;
	}
	case EItemType::Armor:
	{
		UArmorDataAsset* ArmorData = Cast<UArmorDataAsset>(EquipmentItem.ItemData.Get());

		if (!IsValid(ArmorData))																	// 아이템 타입은 Armor지만 방어구 데이터가 아니면 실패 처리
		{
			return false;
		}

		FItemInstance* TargetEquipmentSlot = GetArmorEquipmentSlot(ArmorData->GetArmorType());

		if (TargetEquipmentSlot == nullptr ||
			!IsValid(TargetEquipmentSlot->ItemData) ||
			TargetEquipmentSlot->Quantity <= 0 ||
			TargetEquipmentSlot->ItemData.Get() != EquipmentItem.ItemData.Get())
		{
			return false;
		}

		UInventoryComponent* InventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();

		if (!IsValid(InventoryComponent) ||
			!InventoryComponent->AddItemInstanceToEmptySlot(*TargetEquipmentSlot))					// 빈 인벤토리 슬롯이 없으면 장비 슬롯을 비우지 않는다.
		{
			return false;
		}

		*TargetEquipmentSlot = FItemInstance();
		OnEquipmentChanged.Broadcast();
		return true;
	}
	default:
		return false;
	}
}

bool UEquipmentComponent::UpdateEquippedWeaponState(const FItemInstance& WeaponItem)
{
	UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(WeaponItem.ItemData.Get());

	if (!IsValid(WeaponData) || WeaponItem.Quantity <= 0)
	{
		return false;
	}

	FItemInstance* TargetEquipmentSlot = GetWeaponEquipmentSlot(WeaponData->GetWeaponType());

	if (TargetEquipmentSlot == nullptr ||
		!IsValid(TargetEquipmentSlot->ItemData) ||
		TargetEquipmentSlot->Quantity <= 0 ||
		TargetEquipmentSlot->ItemData.Get() != WeaponItem.ItemData.Get())
	{
		return false;
	}

	*TargetEquipmentSlot = WeaponItem;
	OnEquipmentChanged.Broadcast();
	return true;
}

FItemInstance* UEquipmentComponent::GetWeaponEquipmentSlot(EWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EWeaponType::Sword:
		return &EquippedSword;
	case EWeaponType::Gun:
		return &EquippedGun;
	default:
		return nullptr;
	}
}

FItemInstance* UEquipmentComponent::GetArmorEquipmentSlot(EArmorType ArmorType)
{
	switch (ArmorType)
	{
	case EArmorType::Helmet:
		return &EquippedHelmet;
	case EArmorType::Chestplate:
		return &EquippedChestplate;
	case EArmorType::Leggings:
		return &EquippedLeggings;
	case EArmorType::Boots:
		return &EquippedBoots;
	default:
		return nullptr;
	}
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

float UEquipmentComponent::GetWeaponAttackPowerBonus() const
{
	const AActor* OwnerActor = GetOwner();
	const UWeaponComponent* WeaponComponent =
		IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UWeaponComponent>()
		: nullptr;

	const UWeaponDataAsset* CurrentWeaponData =
		IsValid(WeaponComponent)
		? WeaponComponent->GetCurrentWeaponData()
		: nullptr;

	return IsValid(CurrentWeaponData) ? FMath::Max(0.0f, CurrentWeaponData->GetWeaponPower()) : 0.0f;
}

float UEquipmentComponent::GetArmorDefensePowerBonus() const
{
	return
		GetArmorDefensePowerBonusFromItem(EquippedHelmet) +
		GetArmorDefensePowerBonusFromItem(EquippedChestplate) +
		GetArmorDefensePowerBonusFromItem(EquippedLeggings) +
		GetArmorDefensePowerBonusFromItem(EquippedBoots);
}
