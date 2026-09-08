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

void AMonsterPawnBase::FaceRotation(FRotator NewControlRotation, float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();

	const FRotator TargetRotation(
		0.0f,
		NewControlRotation.Yaw,
		0.0f);

	const FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaTime,
		RotationInterpSpeed);

	SetActorRotation(NewRotation);
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

	FRotator DesiredRotation;
	bool bHasDesiredRotation = false;

	// 공격 등으로 멈춰있을 때도 타겟을 바라보게, 타겟이 있으면 우선 그쪽을 향함
	AActor* TargetActor = MonsterComponent->GetTargetActor();
	if (IsValid(TargetActor))
	{
		const FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
		if (!ToTarget.IsNearlyZero())
		{
			DesiredRotation = ToTarget.Rotation();
			bHasDesiredRotation = true;
		}
	}

	if (!bHasDesiredRotation)
	{
		FVector Velocity = GetVelocity();
		if (!Velocity.IsNearlyZero())
		{
			DesiredRotation = Velocity.Rotation();
			bHasDesiredRotation = true;
		}
	}

	if (bHasDesiredRotation)
	{
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationInterpSpeed);
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
		const float TargetZ = FMath::Max(HitHeights) + HeightAboveFloor;

		// VInterpTo(비율 기반)는 격차가 크면 한 프레임 안에 다 메꿔버려서
		// 바위나 다른 몬스터 위를 지날 때 급격히 튀어 보임. 초당 일정 속도로만
		// 오르내리는 FInterpConstantTo로 바꿔서 격차 크기와 무관하게 부드럽게 이동
		const float NewZ = FMath::FInterpConstantTo(Origin.Z, TargetZ, DeltaTime, HeightCorrectionSpeed);
		const FVector NewLocation(Origin.X, Origin.Y, NewZ);
		SetActorLocation(NewLocation, true);
	}
}
