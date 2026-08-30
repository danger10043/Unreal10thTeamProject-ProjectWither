#include "Monster/MonsterCharacterBase.h"
#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"

AMonsterCharacterBase::AMonsterCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
    MonsterComponent = CreateDefaultSubobject<UMonsterComponent>(TEXT("MonsterComponent"));
}

float AMonsterCharacterBase::TakeDamage(float Damage, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!MonsterComponent || MonsterComponent->IsDead()) return 0.0f;
    const float ReceivedDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    return MonsterComponent->ApplyMonsterDamage(ReceivedDamage);
}

UStatComponent* AMonsterCharacterBase::GetStatComponent_Implementation() const
{
    return StatComponent;
}

void AMonsterCharacterBase::SetMonsterState(EMonsterState NewState)
{
    MonsterComponent->SetMonsterState(NewState);
}

void AMonsterCharacterBase::SetTarget(AActor* NewTarget)
{
    MonsterComponent->SetTarget(NewTarget);
}

void AMonsterCharacterBase::ClearTarget()
{
    MonsterComponent->ClearTarget();
}

AActor* AMonsterCharacterBase::GetTargetActor()
{
    return MonsterComponent->GetTargetActor();
}

float AMonsterCharacterBase::GetDistanceToTarget()
{
    return MonsterComponent->GetDistanceToTarget();
}

void AMonsterCharacterBase::CalculateDrops()
{
    MonsterComponent->CalculateDrops();
}

void AMonsterCharacterBase::DropItems()
{
    MonsterComponent->DropItems();
}
