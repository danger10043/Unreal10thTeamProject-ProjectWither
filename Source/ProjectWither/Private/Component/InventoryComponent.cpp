// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InventoryComponent.h"
#include "DataAsset/ItemDataAsset.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryItems.SetNum(MaxInventorySlot);
}


void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

int32 UInventoryComponent::AddItem(UItemDataAsset* Item, int32 AddQuantity)
{
	if (Item == nullptr || AddQuantity <= 0)
	{
		return 0;
	}

	const int32 ItemId = Item->GetItemId();
	const int32 MaxStack = FMath::Max(1, Item->GetMaxStack());
	int32 RemainingAddQuantity = AddQuantity;
	int32 TotalAddedQuantity = 0;

	// 기존 슬롯부터 채우기
	for (FItemInstance& InventoryItem : InventoryItems)												// 기존 슬롯을 순회하며 아이템을 추가할 수 있는지 확인
	{
		if (InventoryItem.ItemData == nullptr || InventoryItem.ItemData->GetItemId() != ItemId)		// 아이템이 없거나 아이템 ID가 다르면 건너뛴다.
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

		InventoryItem.ItemData = Item;																// 비어있는 슬롯에 아이템 정보 설정
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

bool UInventoryComponent::UseItem(int32 ItemId)
{
	return false;
}


bool UInventoryComponent::HasItem(int32 ItemId) const												// 아이템 보유 여부 반환
{
	return GetItemCount(ItemId) > 0;
}

int32 UInventoryComponent::GetItemCount(int32 ItemId) const
{
	int32 TotalQuantity = 0;
	
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

bool UInventoryComponent::CanAddItem(UItemDataAsset* Item, int32 AddQuantity) const
{
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
	for (FItemInstance& InventoryItem : InventoryItems)												// 모든 슬롯을 순회하며 슬롯 데이터 초기화
	{
		ClearSlotData(InventoryItem);
	}

	OnInventoryChanged.Broadcast();																	// 인벤토리 아이템 목록 변경 이벤트 호출
}
