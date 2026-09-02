#include "Monster/MonsterPawnBase.h"
#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"

AMonsterPawnBase::AMonsterPawnBase()
{
    PrimaryActorTick.bCanEverTick = true;
    StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
    MonsterComponent = CreateDefaultSubobject<UMonsterComponent>(TEXT("MonsterComponent"));
    bUseControllerRotationYaw = false;
}

float AMonsterPawnBase::TakeDamage(float Damage, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!MonsterComponent || MonsterComponent->IsDead()) return 0.0f;
    const float ReceivedDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    return MonsterComponent->ApplyMonsterDamage(ReceivedDamage);
}

UStatComponent* AMonsterPawnBase::GetStatComponent_Implementation() const
{
    return StatComponent;
}

void AMonsterPawnBase::OnSpawnFromPool_Implementation()
{
    if (IsValid(MonsterComponent))
    {
        MonsterComponent->ActivateFromPool();
    }
}

void AMonsterPawnBase::OnReturnToPool_Implementation()
{
    if (IsValid(MonsterComponent))
    {
        MonsterComponent->DeactivateForPool();
    }
}

void AMonsterPawnBase::SetMonsterState(EMonsterState NewState)
{
    MonsterComponent->SetMonsterState(NewState);
}

void AMonsterPawnBase::SetTarget(AActor* NewTarget)
{
    MonsterComponent->SetTarget(NewTarget);
}

void AMonsterPawnBase::ClearTarget()
{
    MonsterComponent->ClearTarget();
}

AActor* AMonsterPawnBase::GetTargetActor()
{
    return MonsterComponent->GetTargetActor();
}

float AMonsterPawnBase::GetDistanceToTarget()
{
    return MonsterComponent->GetDistanceToTarget();
}

void AMonsterPawnBase::CalculateDrops()
{
    MonsterComponent->CalculateDrops();
}

void AMonsterPawnBase::DropItems()
{
    MonsterComponent->DropItems();
}

void AMonsterPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MonsterComponent->IsDead()) return;

	FVector Velocity = GetVelocity();
	if (!Velocity.IsNearlyZero())
	{
		FRotator TargetRotation = Velocity.Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed);
		SetActorRotation(NewRotation);
	}
	if (bSnapToFloor) SnapToFloor(DeltaTime);
}




void AMonsterPawnBase::SnapToFloor(float DeltaTime)
{
	FVector Origin = GetActorLocation();
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	TArray<FVector> TraceOffsets = {
		FVector::ZeroVector,
		Forward * TraceOffsetRadius,
		-Forward * TraceOffsetRadius,
		Right * TraceOffsetRadius,
		-Right * TraceOffsetRadius
	};

	TArray<float> HitHeights;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	for (const FVector& Offset : TraceOffsets)
	{
		FVector Start = Origin + Offset;
		FVector End = Start - FVector(0.f, 0.f, FloorTraceDistance);

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			HitHeights.Add(Hit.Location.Z);
		}
	}

	if (HitHeights.Num() > 0)
	{
		float TargetZ = FMath::Max(HitHeights);
		FVector TargetLocation = FVector(Origin.X, Origin.Y, TargetZ + HeightAboveFloor);
		FVector NewLocation = FMath::VInterpTo(Origin, TargetLocation, DeltaTime, 10.f);
		SetActorLocation(NewLocation, true);
	}
}
