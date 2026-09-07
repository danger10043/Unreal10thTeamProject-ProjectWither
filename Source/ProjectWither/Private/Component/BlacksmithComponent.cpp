// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BlacksmithComponent.h"

#include "Component/InventoryComponent.h"
#include "DataAsset/CraftingRecipeDataAsset.h"
#include "DataAsset/ItemDataAsset.h"

UBlacksmithComponent::UBlacksmithComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ECraftingResult UBlacksmithComponent::CheckCraft(UInventoryComponent* Inventory, UCraftingRecipeDataAsset* Recipe) const
{
	if (!IsValid(Inventory) || !IsValid(Recipe) || !IsValid(Recipe->ResultItem) || Recipe->ResultQuantity <= 0 || Recipe->GoldCost < 0)
	{
		return ECraftingResult::InvalidRecipe;
	}

	TMap<int32, int32> RequiredCounts;

	if (!BuildRequiredMaterialCounts(Recipe, RequiredCounts))
	{
		return ECraftingResult::InvalidRecipe;
	}

	// 필요한 모든 재료 확인
	for (const TPair<int32, int32>& Required : RequiredCounts)
	{
		if (Inventory->GetItemCount(Required.Key) < Required.Value)
		{
			return ECraftingResult::NotEnoughMaterials;
		}
	}

	// 골드 확인
	if (!Inventory->HasEnoughGold(Recipe->GoldCost))
	{
		return ECraftingResult::NotEnoughGold;
	}

	// 결과 아이템을 전부 넣을 수 있는지 확인
	if (!Inventory->CanAddItem(Recipe->ResultItem, Recipe->ResultQuantity))
	{
		return ECraftingResult::InventoryFull;
	}

	return ECraftingResult::Success;
}

ECraftingResult UBlacksmithComponent::Craft(UInventoryComponent* Inventory, UCraftingRecipeDataAsset* Recipe)
{
	const ECraftingResult CheckResult = CheckCraft(Inventory, Recipe);

	if (CheckResult != ECraftingResult::Success)
	{
		return CheckResult;
	}

	// 재료 제거
	for (const FCraftingIngredient& Ingredient : Recipe->RequiredMaterials)
	{
		const int32 ItemId = Ingredient.ItemData->GetItemId();

		if (!Inventory->RemoveItem(ItemId, Ingredient.Quantity))
		{
			/*
			 * CheckCraft 이후 인벤토리가 변경되지 않는 현재
			 * 싱글플레이 구조에서는 일반적으로 발생하지 않는다.
			 */
			return ECraftingResult::TransactionFailed;
		}
	}

	// 골드 제거
	if (!Inventory->SpendGold(Recipe->GoldCost))
	{
		return ECraftingResult::TransactionFailed;
	}

	// 결과 아이템 지급
	const int32 AddedQuantity = Inventory->AddItem(Recipe->ResultItem, Recipe->ResultQuantity);

	if (AddedQuantity != Recipe->ResultQuantity)
	{
		return ECraftingResult::TransactionFailed;
	}

	return ECraftingResult::Success;
}

TArray<UCraftingRecipeDataAsset*> UBlacksmithComponent::GetCraftingRecipes() const
{
	TArray<UCraftingRecipeDataAsset*> Result;

	for (const TObjectPtr<UCraftingRecipeDataAsset>& Recipe : CraftingRecipes)
	{
		if (IsValid(Recipe))
		{
			Result.Add(Recipe.Get());
		}
	}

	return Result;
}

void UBlacksmithComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBlacksmithComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UBlacksmithComponent::BuildRequiredMaterialCounts(UCraftingRecipeDataAsset* Recipe, TMap<int32, int32>& OutRequiredCounts) const
{
	OutRequiredCounts.Reset();

	if (!IsValid(Recipe)) { return false; }

	for (const FCraftingIngredient& Ingredient : Recipe->RequiredMaterials)
	{
		if (!IsValid(Ingredient.ItemData) || Ingredient.Quantity <= 0)
		{
			return false;
		}

		const int32 ItemId = Ingredient.ItemData->GetItemId();

		// 같은 ItemId가 여러 번 들어갔으면 수량을 합친다.
		OutRequiredCounts.FindOrAdd(ItemId) += Ingredient.Quantity;
	}

	return true;
}

