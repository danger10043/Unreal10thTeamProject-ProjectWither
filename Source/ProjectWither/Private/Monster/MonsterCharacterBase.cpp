#include "Monster/MonsterCharacterBase.h"
#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMonsterCharacterBase::AMonsterCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));

    MonsterComponent = CreateDefaultSubobject<UMonsterComponent>(TEXT("MonsterComponent"));

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
		Capsule->SetGenerateOverlapEvents(false);
		Capsule->SetNotifyRigidBodyCollision(false);
		Capsule->CanCharacterStepUpOn = ECB_No;
	}

	if (USkeletalMeshComponent* MonsterMesh = GetMesh())
	{
		MonsterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MonsterMesh->SetCollisionObjectType(ECC_WorldDynamic);
		MonsterMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		MonsterMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		MonsterMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		MonsterMesh->SetGenerateOverlapEvents(true);
		MonsterMesh->SetNotifyRigidBodyCollision(false);
		MonsterMesh->CanCharacterStepUpOn = ECB_No;
	}

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    UCharacterMovementComponent* Movement =
        GetCharacterMovement();

    if (Movement)
    {
        Movement->bOrientRotationToMovement = true;
        Movement->bUseControllerDesiredRotation = false;
        Movement->RotationRate =
            FRotator(0.0f, 180.0f, 0.0f);
    }
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

void AMonsterCharacterBase::OnSpawnFromPool_Implementation()
{
    if (IsValid(MonsterComponent))
    {
        MonsterComponent->ActivateFromPool();
    }
}

void AMonsterCharacterBase::OnReturnToPool_Implementation()
{
    if (IsValid(MonsterComponent))
    {
        MonsterComponent->DeactivateForPool();
    }
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
