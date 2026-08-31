// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Data/ItemDropTable.h"
#include "Item/PickupItem.h"
#include "Monster/MonsterAIController.h"
#include "Player/PlayerCharacter.h"
#include "Interface/StatComponentUserInterface.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"

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
	DisableAllAttackHitboxes();

	for (const auto& Entry : AttackHitboxes)
	{
		if (IsValid(Entry.Value.Get()))
		{
			Entry.Value->OnComponentBeginOverlap.RemoveDynamic(
				this,
				&UMonsterComponent::OnAttackHitboxOverlap);
		}
	}

	AttackHitboxes.Reset();

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
	DisableAllAttackHitboxes();

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

	USkeletalMeshComponent* Mesh =
		GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	UAnimInstance* AnimInstance =
		IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;

	if (!IsValid(AnimInstance)) return;

	const FName SelectedSection = SelectAttackSection();
	if (SelectedSection.IsNone()) return;

	const EMonsterState PreviousState = MonsterState;

	bCanAttack = false;
	SetMonsterState(EMonsterState::Attack);

	const float PlayedLength =
		AnimInstance->Montage_Play(AttackMontage);

	if (PlayedLength <= 0.0f)
	{
		// 재생 실패 시 공격 잠금 복구
		bCanAttack = true;
		SetMonsterState(PreviousState);
		return;
	}

	AnimInstance->Montage_JumpToSection(
		SelectedSection, AttackMontage);

	// 재생에 성공한 경우에만 직전 공격으로 기록
	LastAttackSection = SelectedSection;

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

void UMonsterComponent::RegisterAttackHitbox(FName HitboxName, UPrimitiveComponent* Hitbox)
{
	if (HitboxName.IsNone() || IsValid(Hitbox))
	{
		return;
	}

	if (Hitbox->GetOwner() != GetOwner())
	{
		return;
	}

	if (AttackHitboxes.Contains(HitboxName))
	{
		return;
	}

	// 같은 콜리전을 다른 이름으로 중복 등록 방지
	for (const auto& Entry : AttackHitboxes)
	{
		if (Entry.Value.Get() == Hitbox)
		{
			return;
		}
	}

	Hitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Hitbox->SetGenerateOverlapEvents(true);
	Hitbox->SetCollisionObjectType(ECC_WorldDynamic);
	Hitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	Hitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Hitbox->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UMonsterComponent::OnAttackHitboxOverlap);

	AttackHitboxes.Add(HitboxName, Hitbox);

	UE_LOG(LogTemp, Log, TEXT("공격 콜리전 등록: %s"),
		*HitboxName.ToString());
}

void UMonsterComponent::BeginAttackHitWindow(FName HitboxName)
{
	if (bIsDead || MonsterState != EMonsterState::Attack)
	{
		return;
	}

	// 이전 구간 종료
	DisableAllAttackHitboxes();

	const TObjectPtr<UPrimitiveComponent>* Found =
		AttackHitboxes.Find(HitboxName);

	if (!Found || !IsValid(Found->Get()))
	{
		return;
	}

	UPrimitiveComponent* Hitbox = Found->Get();

	// 활성화 순간 Overlap이 발생할 수 있으므로 먼저 기록
	HitActors.Reset();
	ActiveAttackHitbox = Hitbox;

	Hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 활성화 시 이미 겹쳐 있는 대상도 검사
	TArray<AActor*> OverlappingActors;
	Hitbox->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		ProcessAttackOverlap(Actor);
	}
}

void UMonsterComponent::EndAttackHitWindow(FName HitboxName)
{
	const TObjectPtr<UPrimitiveComponent>* Found =
		AttackHitboxes.Find(HitboxName);

	if (!Found || !IsValid(Found->Get()))
	{
		return;
	}

	UPrimitiveComponent* Hitbox = Found->Get();

	if (ActiveAttackHitbox.Get() == Hitbox)
	{
		ActiveAttackHitbox = nullptr;
		HitActors.Reset();
	}

	Hitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void UMonsterComponent::DisableAllAttackHitboxes()
{
	ActiveAttackHitbox = nullptr;
	HitActors.Reset();

	for (const auto& Entry : AttackHitboxes)
	{
		if (IsValid(Entry.Value.Get()))
		{
			Entry.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void UMonsterComponent::OnAttackHitboxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != ActiveAttackHitbox.Get())
	{
		return;
	}

	ProcessAttackOverlap(OtherActor);
}

void UMonsterComponent::ProcessAttackOverlap(AActor* OtherActor)
{
	if (bIsDead ||
		MonsterState != EMonsterState::Attack ||
		!IsValid(ActiveAttackHitbox))
	{
		return;
	}

	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!IsValid(Player))
	{
		return;
	}

	UStatComponent* PlayerStat =
		IStatComponentUserInterface::Execute_GetStatComponent(Player);

	if (!IsValid(PlayerStat) || PlayerStat->IsHealthZero())
	{
		return;
	}

	const TWeakObjectPtr<AActor> HitActor(OtherActor);

	if (HitActors.Contains(HitActor))
	{
		return;
	}

	HitActors.Add(HitActor);

	UE_LOG(LogTemp, Warning, TEXT("공격 판정: %s → %s"),
		*GetNameSafe(ActiveAttackHitbox.Get()),
		*GetNameSafe(OtherActor));

	// Todo: 실제 피해 적용 
}
