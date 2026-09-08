// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlacksmithComponent.generated.h"


class UInventoryComponent;
class UCraftingRecipeDataAsset;
class UEnhancementDataAsset;

// 제작 요청 결과
UENUM(BlueprintType)
enum class ECraftingResult : uint8
{
	Success             UMETA(DisplayName = "제작 성공"),
	InvalidRecipe       UMETA(DisplayName = "잘못된 레시피"),
	NotEnoughMaterials  UMETA(DisplayName = "재료 부족"),
	NotEnoughGold       UMETA(DisplayName = "골드 부족"),
	InventoryFull       UMETA(DisplayName = "인벤토리 공간 부족"),
	TransactionFailed   UMETA(DisplayName = "제작 처리 실패")
};

UCLASS( ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class PROJECTWITHER_API UBlacksmithComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBlacksmithComponent();

	// 현재 레시피로 제작 가능한지 확인
	UFUNCTION(BlueprintPure, Category = "Blacksmith|Crafting")
	ECraftingResult CheckCraft(UInventoryComponent* Inventory, UCraftingRecipeDataAsset* Recipe) const;

	// 실세 제작 실행
	UFUNCTION(BlueprintCallable, Category = "Blacksmith|Crafting")
	ECraftingResult Craft(UInventoryComponent* Inventory, UCraftingRecipeDataAsset* Recipe);

	// 대장장이가 제공하는 전체 제작 레시피
	UFUNCTION(BlueprintPure, Category = "Blacksmith|Crafting")
	TArray<UCraftingRecipeDataAsset*> GetCraftingRecipes() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 같은 재료가 레시피에 여러 번 들어간 경우 필요한 개수를 하나로 합친다.
	bool BuildRequiredMaterialCounts( UCraftingRecipeDataAsset* Recipe, TMap<int32, int32>& OutRequiredCounts) const;

private:
	// 이 대장장이가 제작할 수 있는 전체 레시피
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blacksmith|Crafting", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UCraftingRecipeDataAsset>> CraftingRecipes;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blacksmith|Enhancement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnhancementDataAsset> EnhancementData;
};
