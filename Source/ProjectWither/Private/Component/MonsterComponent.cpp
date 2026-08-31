// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MonsterComponent.h"
#include "Data/ItemDropTable.h"
#include "Item/PickupItem.h"
#include "Component/StatComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
#include "Monster/MonsterAIController.h"
#include "Animation/AnimMontage.h"

UMonsterComponent::UMonsterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterComponent::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* Owner = GetOwner())
    {
        SpawnLocation = Owner->GetActorLocation();
        StatComponent = Owner->FindComponentByClass<UStatComponent>();
        if (StatComponent)
        {
            StatComponent->OnHealthZero.AddUniqueDynamic(this, &UMonsterComponent::HandleDeath);
        }
    }
}

void UMonsterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(StatComponent))
    {
        StatComponent->OnHealthZero.RemoveDynamic(this, &UMonsterComponent::HandleDeath);
    }
    Super::EndPlay(EndPlayReason);
}

float UMonsterComponent::ApplyMonsterDamage(float Damage)
{
    if (bIsDead || !IsValid(StatComponent)) return 0.0f;
    const float AppliedDamage = StatComponent->ApplyDamage(Damage);
    if (StatComponent->IsHealthZero()) HandleDeath();
    return AppliedDamage;
}

void UMonsterComponent::HandleDeath()
{
    if (bIsDead) return;
    bIsDead = true;
    MonsterState = EMonsterState::Dead;
    ClearTarget();

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AMonsterAIController* MonsterAI =
			Cast<AMonsterAIController>(Pawn->GetController()))
		{
			MonsterAI->StopAI();
		}
		else if (AAIController* AI =
			Cast<AAIController>(Pawn->GetController()))
		{
			// 다른 AIController를 사용하는 경우의 기본 처리
			if (UBrainComponent* Brain = AI->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Monster died"));
			}

			AI->StopMovement();
		}
	}
    CalculateDrops();
    DropItems();
    OnMonsterDied.Broadcast();
}

void UMonsterComponent::SetMonsterState(EMonsterState NewState)
{
	if (!bIsDead) MonsterState = NewState;
}

void UMonsterComponent::SetTarget(AActor* NewTarget)
{
	if (!bIsDead) TargetActor = NewTarget;
}

void UMonsterComponent::ClearTarget()
{
	TargetActor = nullptr;
}

AActor* UMonsterComponent::GetTargetActor() const
{
	return IsValid(TargetActor) ? TargetActor.Get() : nullptr;
}

float UMonsterComponent::GetDistanceToTarget() const
{
	if (!IsValid(TargetActor) || !GetOwner()) return -1.0f;

	return GetOwner()->GetDistanceTo(TargetActor);
}

void UMonsterComponent::CalculateDrops()
{
	DropItem.Reset();
	if (!ItemDropTable) return;

	TArray<FItemDropTable*> AllRows;
	ItemDropTable->GetAllRows<FItemDropTable>(
		TEXT("UMonsterComponent::CalculateDrops"),
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

void UMonsterComponent::DropItems()
{
	if (!ItemPickupClass || !GetOwner()) return;

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
			GetOwner()->GetActorLocation() +
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
			FItemInstance DroppedItem;
			DroppedItem.ItemData = Drop.Key;
			DroppedItem.Quantity = Drop.Value;

			Pickup->InitializePickup(DroppedItem);
		}
	}

	DropItem.Reset();
}

bool UMonsterComponent::IsInAttackRange()
{
	if (!IsValid(TargetActor) || !IsValid(GetOwner()))
	{
		return false;
	}

	const float Distance = GetDistanceToTarget();

	return Distance >= 0.0f && Distance <= AttackRange;
}

bool UMonsterComponent::CanAttack()
{
	if (bIsDead || !bCanAttack)
	{
		return false;
	}

	if (!IsValid(StatComponent) || StatComponent->IsHealthZero())
	{
		return false;
	}

	if (MonsterState == EMonsterState::Attack ||
		MonsterState == EMonsterState::Hit ||
		MonsterState == EMonsterState::Dead)
	{
		return false;
	}

	return IsInAttackRange();
}

void UMonsterComponent::Attack()
{
	if (!CanAttack() || !IsValid(AttackMontage))
	{
		return;
	}



	bCanAttack = false;
	SetMonsterState(EMonsterState::Attack);

}

void UMonsterComponent::FinishAttack()
{
}

void UMonsterComponent::ResetAttackCooldown()
{
}

void UMonsterComponent::ApplyAttackDamage()
{
}

FName UMonsterComponent::SelectAttackSection() const
{
	if (!IsValid(AttackMontage))
	{
		return NAME_None;
	}

	TArray<FName> Candidates;

	for (int32 Index = 0; Index < AttackMontage->GetNumSections(); Index++)
	{
		const FName SectionName = AttackMontage->GetSectionName(Index);

		if (!SectionName.ToString().StartsWith(AttackSectionPrefix))
		{
			continue;
		}

		if (SectionName == LastAttackSection)
		{
			continue;
		}

		Candidates.Add(SectionName);
	}

	if (Candidates.IsEmpty())
	{
		return NAME_None;
	}

	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}
