// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemInstance.h"
#include "InventoryComponent.generated.h"

class UItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChangedDelegate);														// 인벤토리 아이템 목록이 변경되었을 때 호출되는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoldChangedDelegate, int32, CurrentGold, int32, ChangedAmount);			// 골드가 변경되었을 때 현재 골드와 변경량을 전달하는 델리게이트

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTWITHER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")										// 바인딩 가능한 인벤토리 변경 이벤트 델리게이트
	FOnInventoryChangedDelegate OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")										// 바인딩 가능한 골드 변경 이벤트 델리게이트
	FOnGoldChangedDelegate OnGoldChanged;


	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(UItemDataAsset* Item, int32 AddQuantity);											// 아이템을 인벤토리에 추가하고 실제 추가된 수량을 반환

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 ItemId, int32 RemoveQuantity);											// 인벤토리 전체에서 지정한 아이템을 수량만큼 제거 (강화 재료, 소비 아이템 등에서 사용)

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemAtSlot(int32 SlotIndex, int32 RemoveQuantity);									// 지정한 슬롯에서만 아이템 수량 제거 (아이템 장착, 아이템을 버릴 때 등)

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SwapItems(int32 FromSlotIndex, int32 ToSlotIndex);										// 두 슬롯의 아이템 위치를 교환

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItemAtSlot(int32 SlotIndex);															// 지정한 슬롯의 아이템을 사용

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 ItemId);																		// 지정한 아이템을 사용

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(int32 ItemId) const;																// 지정한 아이템 보유 여부 반환

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(int32 ItemId) const;															// 지정한 아이템의 총 보유 수량 반환

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FItemInstance> GetInventoryItems() const;												// UI 표시용 현재 인벤토리 아이템 목록 반환

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetItemAtSlot(int32 SlotIndex, FItemInstance& OutItem) const;								// 지정한 슬롯의 아이템 정보를 반환

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetMaxInventorySlot() const { return MaxInventorySlot; }						// 최대 인벤토리 슬롯 수 반환

	const FItemInstance* FindItem(int32 ItemId) const;												// 지정한 아이템 정보를 찾아 C++ 전용 읽기 포인터 반환

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanAddItem(UItemDataAsset* Item, int32 AddQuantity) const;									// 지정된 수량을 인벤토리에 모두 추가할 수 있는지 확인

	UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
	void AddGold(int32 Amount);																		// 골드 증가

	UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
	bool SpendGold(int32 Amount);																	// 골드가 충분하면 지정한 액수만큼 사용


	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearInventory();																			// 인벤토리 전체 초기화

	UFUNCTION(BlueprintPure, Category = "Inventory|Gold")
	FORCEINLINE bool HasEnoughGold(int32 Amount) const { return Amount >= 0 && Gold >= Amount; }	// 가지고 있는 골드가 지정한 금액에 대해 충분한 지 반환

	UFUNCTION(BlueprintPure, Category = "Inventory|Gold")
	FORCEINLINE int32 GetGold() const { return Gold; }												// 현재 보유 골드 반환

private:
	void ClearSlotData(FItemInstance& InventoryItem);												// 슬롯 하나의 아이템 데이터 초기화

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))		// 현재 보유 아이템 목록 배열
	TArray<FItemInstance> InventoryItems;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
	int32 MaxInventorySlot = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 Gold = 0;
};
