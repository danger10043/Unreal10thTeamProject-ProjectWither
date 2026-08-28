// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterBase.h"
#include "Data/ItemDropTable.h"
#include "Item/PickupItem.h"
#include "Component/StatComponent.h"

// Sets default values
AMonsterBase::AMonsterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AMonsterBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead || !StatComponent)
	{
		return 0.0f;
	}

	float ReceivedDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	const float AppliedDamage = StatComponent->ApplyDamage(ReceivedDamage);

	if (StatComponent->IsHealthZero())
	{
		bIsDead = true;

		CalculateDrops();
		DropItems();
	}

	return AppliedDamage;
}

void AMonsterBase::SetMonsterState(EMonsterState NewState)
{
	MonsterState = NewState;
}

void AMonsterBase::SetTarget(AActor* NewTarget)
{
	TargetActor = NewTarget;
}

void AMonsterBase::ClearTarget()
{
	TargetActor = nullptr;
}

AActor* AMonsterBase::GetTargetActor()
{
	return TargetActor;
}

float AMonsterBase::GetDistanceToTarget()
{
	if (!TargetActor) return InValidTargetActor;

	return GetDistanceTo(TargetActor);
}

void AMonsterBase::CalculateDrops()
{
	if (!ItemDropTable) return;

	DropItem.Reset();	// 혹시나 남아있을 드랍 아이템 목록 초기화

	TArray<FItemDropTable*> AllRows;
	ItemDropTable->GetAllRows<FItemDropTable>(
		TEXT("AMonsterBase::CalculateDrops"),
		AllRows
	);
	for (FItemDropTable* Row : AllRows)
	{
		// 필수 데이터 확인
		if (!Row || !Row->ItemData) continue;

		// 드랍 확률 체크
		if (FMath::FRand() > Row->DropRate) continue;

		int32 Quantity = FMath::RandRange(Row->MinQuantity, Row->MaxQuantity);
		
		DropItem.FindOrAdd(Row->ItemData) += Quantity;
	}
}

void AMonsterBase::DropItems()
{
	if (!ItemPickupClass) return;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TPair<TObjectPtr<UItemDataAsset>, int32>& Drop : DropItem)
	{
		if (!Drop.Key || Drop.Value <= 0)
		{
			continue;
		}

		const FVector DropLocation =
			GetActorLocation() +
			FVector(
				FMath::RandRange(-DropRange, DropRange),
				FMath::RandRange(-DropRange, DropRange),
				DropHeight
			);

		APickupItem* Pickup = World->SpawnActor<APickupItem>(
			ItemPickupClass,
			DropLocation,
			FRotator::ZeroRotator
		);

		if (Pickup)
		{
			Pickup->InitializePickup(Drop.Key, Drop.Value);
		}
	}

	DropItem.Reset();
}
