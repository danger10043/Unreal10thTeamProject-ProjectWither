// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InventoryComponent.h"
#include "Equipment/EquipmentComponent.h"
#include "Component/StatComponent.h"
#include "DataAsset/ItemDataAsset.h"
#include "DataAsset/PotionDataAsset.h"
#include "GameFramework/Actor.h"
#include "Interface/EquipmentComponentUserInterface.h"
#include "Interface/StatComponentUserInterface.h"
#include "DataAsset/AmmoDataAsset.h"
#include "DataAsset/WeaponDataAsset.h"
#include "UObject/ConstructorHelpers.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UPotionDataAsset> DefaultPotionAsset(
		TEXT("/Game/Main/DataAsset/Item/1_UseItem/1_Potion/DA_ID1101_Healpack.DA_ID1101_Healpack"));
	if (DefaultPotionAsset.Succeeded())
	{
		PotionItem.ItemData = DefaultPotionAsset.Object;
	}
}

namespace
{
	bool CanStackItemInstances(const FItemInstance& ExistingItem, const FItemInstance& NewItem)
	{
		return
			IsValid(ExistingItem.ItemData) &&
			IsValid(NewItem.ItemData) &&
			ExistingItem.ItemData->GetItemId() == NewItem.ItemData->GetItemId() &&
			ExistingItem.EnhanceLevel == NewItem.EnhanceLevel &&
			ExistingItem.CurrentAmmo == NewItem.CurrentAmmo;
	}
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryItems.SetNum(MaxInventorySlot);
	MigrateInventoryPotions();
	PotionItem.Quantity = FMath::Clamp(PotionItem.Quantity, 0, GetMaxPotionQuantity());
	OnPotionChanged.Broadcast(PotionItem.Quantity, GetMaxPotionQuantity());
}


void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

int32 UInventoryComponent::AddItem(UItemDataAsset* Item, int32 AddQuantity)
{
	FItemInstance NewItemInstance;
	NewItemInstance.ItemData = Item;
	NewItemInstance.Quantity = AddQuantity;

	return AddItemInstance(NewItemInstance);
}

int32 UInventoryComponent::AddItemInstance(const FItemInstance& NewItemInstance)
{
	if (!IsValid(NewItemInstance.ItemData) || NewItemInstance.Quantity <= 0) return 0;

	if (NewItemInstance.ItemData->GetItemType() == EItemType::Potion)
	{
		return AddPotion(Cast<UPotionDataAsset>(NewItemInstance.ItemData.Get()), NewItemInstance.Quantity);
	}

	const int32 MaxStack = FMath::Max(1, NewItemInstance.ItemData->GetMaxStack());
	int32 RemainingAddQuantity = NewItemInstance.Quantity;
	int32 TotalAddedQuantity = 0;

	// 기존 슬롯부터 채우기
	for (FItemInstance& InventoryItem : InventoryItems)												// 기존 슬롯을 순회하며 아이템을 추가할 수 있는지 확인
	{
		if (!CanStackItemInstances(InventoryItem, NewItemInstance))									// 아이템 ID와 인스턴스 상태가 다르면 같은 스택으로 합치지 않는다.
		{
			continue;
		}

		const int32 AddableQuantity = FMath::Max(0, MaxStack - InventoryItem.Quantity);				// 현재 슬롯에 추가할 수 있는 수량
		const int32 AddedQuantity = FMath::Min(RemainingAddQuantity, AddableQuantity);				// 추가할 수 있는 수량과 남은 수량 중 작은 값

		InventoryItem.Quantity += AddedQuantity;													// 같은 슬롯이면 보유 수량 증가
		RemainingAddQuantity -= AddedQuantity;														// 추가해야 할 남은 수량 갱신
		TotalAddedQuantity += AddedQuantity;														// 실제로 추가된 수량 갱신

		if (RemainingAddQuantity <= 0)																// 요청 수량을 모두 추가했으면 종료
		{
			OnInventoryChanged.Broadcast();															// 인벤토리 아이템 목록 변경 이벤트 호출
			return TotalAddedQuantity;
		}
	}

	// 기존 슬롯으로 부족하면 비어있는 슬롯에 최대 스택 단위로 나누어 추가
	for (FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData != nullptr)														// 이미 아이템이 들어있는 슬롯이면 건너뛴다.
		{
			continue;
		}

		InventoryItem = NewItemInstance;															// 비어있는 슬롯에 인스턴스 상태까지 포함한 아이템 정보 설정
		InventoryItem.Quantity = FMath::Min(RemainingAddQuantity, MaxStack);						// 한 슬롯에 들어갈 수 있는 수량만큼 추가

		RemainingAddQuantity -= InventoryItem.Quantity;												// 추가해야 할 남은 수량 감소
		TotalAddedQuantity += InventoryItem.Quantity;												// 실제로 추가된 수량 증가

		if (RemainingAddQuantity <= 0)																// 요청 수량을 모두 추가했으면 종료
		{
			OnInventoryChanged.Broadcast();															// 인벤토리 아이템 목록 변경 이벤트 호출
			return TotalAddedQuantity;
		}
	}

	// 요청 수량을 전부 넣지 못해도 실제로 추가된 개수를 반환
	if (TotalAddedQuantity > 0)																		// 일부라도 추가되었으면 인벤토리 변경 이벤트 호출
	{
		OnInventoryChanged.Broadcast();
	}

	return TotalAddedQuantity;
}

bool UInventoryComponent::RemoveItem(int32 ItemId, int32 RemoveQuantity)
{
	if (IsValid(PotionItem.ItemData) && PotionItem.ItemData->GetItemId() == ItemId)
	{
		if (RemoveQuantity <= 0 || PotionItem.Quantity < RemoveQuantity) return false;
		PotionItem.Quantity -= RemoveQuantity;
		OnPotionChanged.Broadcast(PotionItem.Quantity, GetMaxPotionQuantity());
		return true;
	}

	if (RemoveQuantity <= 0)																		// 제거 요청 수량이 0 이하이면 실패 처리
	{
		return false;
	}

	int32 TotalItemQuantity = 0;

	// 인벤토리 전체에서 같은 아이템의 총 보유 수량을 먼저 확인
	for (const FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)		// 아이템이 없거나 아이템 ID가 다르면 건너뛴다.
		{
			continue;
		}

		TotalItemQuantity += FMath::Max(0, InventoryItem.Quantity);									// 현재 보유 중인 아이템 총 수량 계산
	}

	if (TotalItemQuantity < RemoveQuantity)															// 보유 수량이 제거 요청 수량보다 적으면 제거하지 않는다.
	{
		return false;
	}

	// 충분히 보유하고 있을 때만 실제 제거를 시작
	for (FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)		// 아이템이 없거나 아이템 ID가 다르면 건너뛴다.
		{
			continue;
		}

		const int32 RemovableQuantity = FMath::Min(RemoveQuantity, InventoryItem.Quantity);			// 현재 슬롯에서 제거할 수 있는 수량
		InventoryItem.Quantity -= RemovableQuantity;												// 보유 수량 감소
		RemoveQuantity -= RemovableQuantity;														// 제거해야 할 남은 수량 갱신

		if (InventoryItem.Quantity <= 0)																// 슬롯의 수량이 0이 되면 아이템 정보 초기화
		{
			ClearSlotData(InventoryItem);
		}

		if (RemoveQuantity <= 0)																		// 요청 수량을 모두 제거했으면 종료
		{
			OnInventoryChanged.Broadcast();															// 인벤토리 아이템 목록 변경 이벤트 호출
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::RemoveItemAtSlot(int32 SlotIndex, int32 RemoveQuantity)
{
	if (!InventoryItems.IsValidIndex(SlotIndex) || RemoveQuantity <= 0)								// 슬롯 번호가 잘못됐거나 제거 요청 수량이 0 이하이면 실패 처리
	{
		return false;
	}

	FItemInstance& InventoryItem = InventoryItems[SlotIndex];

	if (InventoryItem.ItemData == nullptr || InventoryItem.Quantity < RemoveQuantity)					// 빈 슬롯이거나 슬롯 보유 수량보다 많이 제거하려 하면 실패 처리
	{
		return false;
	}

	InventoryItem.Quantity -= RemoveQuantity;														// 선택한 슬롯에서만 보유 수량 감소

	if (InventoryItem.Quantity <= 0)																	// 슬롯의 수량이 0이 되면 아이템 정보 초기화
	{
		ClearSlotData(InventoryItem);
	}

	OnInventoryChanged.Broadcast();																	// 인벤토리 아이템 목록 변경 이벤트 호출
	return true;
}

bool UInventoryComponent::SwapItems(int32 FromSlotIndex, int32 ToSlotIndex)
{
	if (!InventoryItems.IsValidIndex(FromSlotIndex) || !InventoryItems.IsValidIndex(ToSlotIndex))	// 슬롯 번호가 잘못되었으면 실패 처리
	{
		return false;
	}

	if (FromSlotIndex == ToSlotIndex)																// 같은 슬롯이면 변경할 내용이 없다.
	{
		return false;
	}

	if (InventoryItems[FromSlotIndex].ItemData == nullptr)											// 비어있는 슬롯에서는 아이템 이동을 시작할 수 없다.
	{
		return false;
	}

	InventoryItems.Swap(FromSlotIndex, ToSlotIndex);												// 대상 슬롯이 비어 있으면 이동, 아이템이 있으면 서로 교환
	OnInventoryChanged.Broadcast();																	// 인벤토리 아이템 목록 변경 이벤트 호출
	return true;
}

bool UInventoryComponent::UseItemAtSlot(int32 SlotIndex)
{
	if (!InventoryItems.IsValidIndex(SlotIndex))													// 슬롯 번호가 잘못되었으면 실패 처리
	{
		return false;
	}

	FItemInstance& InventoryItem = InventoryItems[SlotIndex];

	if (InventoryItem.ItemData == nullptr || InventoryItem.Quantity <= 0)							// 빈 슬롯이거나 보유 수량이 없으면 실패 처리
	{
		return false;
	}

	switch (InventoryItem.ItemData->GetItemType())
	{
	case EItemType::Potion:
	{
		UPotionDataAsset* PotionData = Cast<UPotionDataAsset>(InventoryItem.ItemData.Get());

		if (!IsValid(PotionData))																	// 아이템 타입은 Potion이지만 포션 데이터가 아니면 실패 처리
		{
			return false;
		}

		AActor* OwnerActor = GetOwner();

		if (!IsValid(OwnerActor) ||
			!OwnerActor->GetClass()->ImplementsInterface(UStatComponentUserInterface::StaticClass()))
		{
			return false;
		}

		UStatComponent* StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor);

		if (!IsValid(StatComponent))
		{
			return false;
		}

		const float HealPercent = PotionData->GetHealAmount() / 100.0f;
		const float HealAmount = StatComponent->GetMaxHealth() * HealPercent;
		StatComponent->RecoverHealth(HealAmount);									// 최대 체력의 해당 퍼센트로 포션 효과를 적용한다.
		return RemoveItemAtSlot(SlotIndex, 1);										// 포션 사용 처리가 끝나면 포션 1개를 소비한다.
	}
	case EItemType::Weapon:
	case EItemType::Armor:
	{
		AActor* OwnerActor = GetOwner();

		if (!IsValid(OwnerActor) ||
			!OwnerActor->GetClass()->ImplementsInterface(UEquipmentComponentUserInterface::StaticClass()))
		{
			return false;
		}

		UEquipmentComponent* EquipmentComponent = IEquipmentComponentUserInterface::Execute_GetEquipmentComponent(OwnerActor);

		if (!IsValid(EquipmentComponent))
		{
			return false;
		}

		return EquipmentComponent->EquipItemFromInventorySlot(SlotIndex);
	}
	default:
		return false;
	}
}

bool UInventoryComponent::UseItem(int32 ItemId)
{
	if (IsValid(PotionItem.ItemData) && PotionItem.ItemData->GetItemId() == ItemId)
	{
		return UsePotion();
	}

	return false;
}

bool UInventoryComponent::UsePotion()
{
	UPotionDataAsset* PotionData = GetPotionData();
	AActor* OwnerActor = GetOwner();

	if (!IsValid(PotionData) || PotionItem.Quantity <= 0 || !IsValid(OwnerActor) ||
		!OwnerActor->GetClass()->ImplementsInterface(UStatComponentUserInterface::StaticClass()))
	{
		return false;
	}

	UStatComponent* StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor);
	if (!IsValid(StatComponent) || StatComponent->IsHealthZero() ||
		StatComponent->GetCurrentHealth() >= StatComponent->GetMaxHealth())
	{
		return false;
	}

	const float HealAmount = StatComponent->GetMaxHealth() * (PotionData->GetHealAmount() / 100.0f);
	if (StatComponent->RecoverHealth(HealAmount) <= 0.0f) return false;

	--PotionItem.Quantity;
	OnPotionChanged.Broadcast(PotionItem.Quantity, GetMaxPotionQuantity());
	return true;
}

UPotionDataAsset* UInventoryComponent::GetPotionData() const
{
	return Cast<UPotionDataAsset>(PotionItem.ItemData.Get());
}

int32 UInventoryComponent::GetMaxPotionQuantity() const
{
	const UPotionDataAsset* PotionData = GetPotionData();
	return IsValid(PotionData) ? FMath::Max(1, PotionData->GetMaxStack()) : 0;
}

int32 UInventoryComponent::AddPotion(UPotionDataAsset* InPotionData, int32 AddQuantity)
{
	if (!IsValid(InPotionData) || AddQuantity <= 0) return 0;

	UPotionDataAsset* CurrentPotionData = GetPotionData();
	if (IsValid(CurrentPotionData) && CurrentPotionData->GetItemId() != InPotionData->GetItemId()) return 0;

	PotionItem.ItemData = InPotionData;
	const int32 PreviousQuantity = PotionItem.Quantity;
	PotionItem.Quantity = FMath::Clamp(PreviousQuantity + AddQuantity, 0, GetMaxPotionQuantity());
	const int32 AddedQuantity = PotionItem.Quantity - PreviousQuantity;

	if (AddedQuantity > 0)
	{
		OnPotionChanged.Broadcast(PotionItem.Quantity, GetMaxPotionQuantity());
	}

	return AddedQuantity;
}

void UInventoryComponent::MigrateInventoryPotions()
{
	for (FItemInstance& InventoryItem : InventoryItems)
	{
		UPotionDataAsset* PotionData = Cast<UPotionDataAsset>(InventoryItem.ItemData.Get());
		if (!IsValid(PotionData) || InventoryItem.Quantity <= 0) continue;

		AddPotion(PotionData, InventoryItem.Quantity);
		ClearSlotData(InventoryItem);
	}
}


bool UInventoryComponent::HasItem(int32 ItemId) const												// 아이템 보유 여부 반환
{
	return GetItemCount(ItemId) > 0;
}

int32 UInventoryComponent::GetItemCount(int32 ItemId) const
{
	int32 TotalQuantity = IsValid(PotionItem.ItemData) && PotionItem.ItemData->GetItemId() == ItemId
		? PotionItem.Quantity
		: 0;
	
	for (const FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)		// 아이템이 없거나 아이템 ID가 다르면 건너뛴다.
		{
			continue;
		}
		TotalQuantity += InventoryItem.Quantity;													// 현재 슬롯의 수량을 총 수량에 더함
	}
	return TotalQuantity;																			// 같은 아이템이 있으면 수량 반환
}	

TArray<FItemInstance> UInventoryComponent::GetInventoryItems() const
{
	return InventoryItems;
}

bool UInventoryComponent::GetItemAtSlot(int32 SlotIndex, FItemInstance& OutItem) const
{
	if (!InventoryItems.IsValidIndex(SlotIndex))
	{
		OutItem = FItemInstance();
		return false;
	}

	OutItem = InventoryItems[SlotIndex];
	return true;
}

const FItemInstance* UInventoryComponent::FindItem(int32 ItemId) const
{
	for (const FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)
		{
			continue;
		}
		return &InventoryItem;																		// 같은 아이템이 있으면 해당 슬롯의 아이템 정보 반환 (같은 아이템이 여러 슬롯에 있을 경우 가장 먼저 발견한 아이템의 정보만 반환)
	}
	return nullptr;
}

int32 UInventoryComponent::FindItemSlot(int32 ItemId) const
{
	for (int32 SlotIndex = 0; SlotIndex < InventoryItems.Num(); ++SlotIndex)
	{
		const FItemInstance& InventoryItem = InventoryItems[SlotIndex];

		if (IsValid(InventoryItem.ItemData) &&
			InventoryItem.Quantity > 0 &&
			InventoryItem.ItemData->GetItemId() == ItemId)
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UInventoryComponent::FindWeaponSlotByType(EWeaponType WeaponType) const
{
	for (int32 SlotIndex = 0; SlotIndex < InventoryItems.Num(); ++SlotIndex)
	{
		const FItemInstance& InventoryItem = InventoryItems[SlotIndex];

		if (InventoryItem.Quantity <= 0) continue;

		const UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(InventoryItem.ItemData.Get());
		
		if (IsValid(WeaponData) && WeaponData->GetWeaponType() == WeaponType) return SlotIndex;
	}

	return INDEX_NONE;
}

bool UInventoryComponent::UpdataItemAtSlot(int32 SlotIndex, const FItemInstance& NewItemInstance)
{
	if (!InventoryItems.IsValidIndex(SlotIndex)) return false;
	if (!IsValid(NewItemInstance.ItemData)) return false;
	if (NewItemInstance.Quantity <= 0) return false;

	FItemInstance& InventoryItem = InventoryItems[SlotIndex];

	if (!IsValid(InventoryItem.ItemData)) return false;
	if (InventoryItem.ItemData->GetItemId() != NewItemInstance.ItemData->GetItemId()) return false;

	InventoryItem = NewItemInstance;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::SetItemAtSlot(int32 SlotIndex, const FItemInstance& NewItemInstance)
{
	if (!InventoryItems.IsValidIndex(SlotIndex)) return false;

	FItemInstance& InventoryItem = InventoryItems[SlotIndex];

	if (!IsValid(NewItemInstance.ItemData) || NewItemInstance.Quantity <= 0)
	{
		ClearSlotData(InventoryItem);																// 빈 아이템 정보가 들어오면 해당 슬롯을 비운다.
	}
	else
	{
		InventoryItem = NewItemInstance;															// 유효한 아이템 정보가 들어오면 해당 슬롯을 새 아이템으로 교체한다.
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::AddItemInstanceToEmptySlot(const FItemInstance& NewItemInstance)
{
	if (!IsValid(NewItemInstance.ItemData) || NewItemInstance.Quantity <= 0) return false;

	for (FItemInstance& InventoryItem : InventoryItems)											// 장비 해제 시 아이템 고유 상태를 유지한 채 빈 슬롯으로 되돌린다.
	{
		if (IsValid(InventoryItem.ItemData) && InventoryItem.Quantity > 0)
		{
			continue;
		}

		InventoryItem = NewItemInstance;
		OnInventoryChanged.Broadcast();
		return true;
	}

	return false;
}

int32 UInventoryComponent::ConsumeAmmoByType(EAmmoType AmmoType, int32 RequestedQuantity)
{
	if (RequestedQuantity <= 0) return 0;

	int32 RemainingQuantity = RequestedQuantity;
	int32 ConsumedQuantity = 0;

	for (FItemInstance& InventoryItem : InventoryItems)
	{
		UAmmoDataAsset* AmmoData = Cast<UAmmoDataAsset>(InventoryItem.ItemData.Get());

		if (!IsValid(AmmoData) ||
			AmmoData->GetAmmoType() != AmmoType ||
			InventoryItem.Quantity <= 0)
		{
			continue;
		}

		const int32 QuantityFromSlot = FMath::Min(InventoryItem.Quantity, RemainingQuantity);

		InventoryItem.Quantity -= QuantityFromSlot;
		RemainingQuantity -= QuantityFromSlot;
		ConsumedQuantity += QuantityFromSlot;

		if (InventoryItem.Quantity <= 0)
		{
			ClearSlotData(InventoryItem);
		}

		if (RemainingQuantity <= 0)
		{
			break;
		}
	}

	if (ConsumedQuantity > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return ConsumedQuantity;
}

bool UInventoryComponent::CanAddItem(UItemDataAsset* Item, int32 AddQuantity) const
{
	if (IsValid(Item) && Item->GetItemType() == EItemType::Potion)
	{
		const UPotionDataAsset* IncomingPotion = Cast<UPotionDataAsset>(Item);
		const UPotionDataAsset* CurrentPotion = GetPotionData();
		return AddQuantity > 0 && IsValid(IncomingPotion) &&
			(!IsValid(CurrentPotion) || CurrentPotion->GetItemId() == IncomingPotion->GetItemId()) &&
			PotionItem.Quantity + AddQuantity <= FMath::Max(1, IncomingPotion->GetMaxStack());
	}

	if (Item == nullptr || AddQuantity <= 0)
	{
		return false;
	}

	const int32 ItemId = Item->GetItemId();
	const int32 MaxStack = FMath::Max(1, Item->GetMaxStack());
	int32 RequiredAddQuantity = AddQuantity;														// 아직 확보해야 하는 수용 가능 수량

	// 기존 같은 아이템 슬롯에 남아있는 공간부터 확인
	for (const FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)		// 아이템이 없거나 아이템 ID가 다르면 건너뛴다.
		{
			continue;
		}

		RequiredAddQuantity -= FMath::Max(0, MaxStack - InventoryItem.Quantity);					// 현재 슬롯에 더 넣을 수 있는 수량만큼 필요 수량 감소

		if (RequiredAddQuantity <= 0)																// 요청 수량을 모두 담을 수 있으면 true 반환
		{
			return true;
		}
	}

	// 기존 슬롯으로 부족하면 비어있는 슬롯이 담을 수 있는 수량을 확인
	for (const FItemInstance& InventoryItem : InventoryItems)
	{
		if (InventoryItem.ItemData != nullptr)														// 이미 아이템이 들어있는 슬롯이면 건너뛴다.
		{
			continue;
		}

		RequiredAddQuantity -= MaxStack;															// 빈 슬롯 하나가 담을 수 있는 최대 수량만큼 필요 수량 감소

		if (RequiredAddQuantity <= 0)																// 요청 수량을 모두 담을 수 있으면 성공
		{
			return true;
		}
	}
	return false;																					// 모든 슬롯을 확인해도 공간이 부족하면 False 반환
}

void UInventoryComponent::AddGold(int32 Amount)
{
	if (Amount <= 0)																				// 추가할 골드가 0 이하이면 처리하지 않는다.
	{
		return;
	}

	Gold += Amount;																					// 현재 보유 골드 증가
	OnGoldChanged.Broadcast(Gold, Amount);															// 골드 변경 이벤트 호출
}

bool UInventoryComponent::SpendGold(int32 Amount)
{
	if (Amount <= 0 || !HasEnoughGold(Amount))														// 사용 요청 골드가 0 이하이거나 보유 골드가 부족하면 실패 처리
	{
		return false;
	}

	Gold -= Amount;																					// 현재 보유 골드 감소
	OnGoldChanged.Broadcast(Gold, -Amount);															// 골드 변경 이벤트 호출
	return true;
}

void UInventoryComponent::ClearSlotData(FItemInstance& InventoryItem)
{
	InventoryItem.ItemData = nullptr;
	InventoryItem.Quantity = 0;
	InventoryItem.EnhanceLevel = 0;
	InventoryItem.CurrentAmmo = 0;
}

void UInventoryComponent::ClearInventory()
{
	PotionItem.Quantity = 0;
	OnPotionChanged.Broadcast(PotionItem.Quantity, GetMaxPotionQuantity());

	for (FItemInstance& InventoryItem : InventoryItems)												// 모든 슬롯을 순회하며 슬롯 데이터 초기화
	{
		ClearSlotData(InventoryItem);
	}

	OnInventoryChanged.Broadcast();																	// 인벤토리 아이템 목록 변경 이벤트 호출
}

int32 UInventoryComponent::RefillPotionsToMax()
{
	MigrateInventoryPotions();
	const int32 MaxQuantity = GetMaxPotionQuantity();
	const int32 RefilledQuantity = FMath::Max(0, MaxQuantity - PotionItem.Quantity);
	PotionItem.Quantity = MaxQuantity;

	if (RefilledQuantity > 0)
	{
		OnPotionChanged.Broadcast(PotionItem.Quantity, MaxQuantity);
	}

	return RefilledQuantity;
}
