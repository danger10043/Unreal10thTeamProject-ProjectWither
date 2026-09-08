// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/MonsterProjectile.h"
#include "Component/MonsterComponent.h"
#include "Interface/PlayerInterface.h"
#include "Interface/StatComponentUserInterface.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMonsterProjectile::AMonsterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMonsterProjectile::OnProjectileHit);
	SetRootComponent(CollisionComponent);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	SetReplicates(false);
}

void AMonsterProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(ProjectileLifetime);
}

void AMonsterProjectile::InitializeProjectile(
	UMonsterComponent* InSourceMonsterComponent,
	const FVector& LaunchDirection,
	float InAttackMultiplier)
{
	SourceMonsterComponent = InSourceMonsterComponent;
	AttackMultiplier = InAttackMultiplier;

	if (AActor* Shooter = GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(Shooter, true);
	}

	const FVector SafeDirection = LaunchDirection.GetSafeNormal();
	if (!SafeDirection.IsNearlyZero())
	{
		ProjectileMovement->Velocity = SafeDirection * ProjectileMovement->InitialSpeed;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("InitializeProjectile: SpawnLocation=%s, Velocity=%s (Speed=%.1f)"),
		*GetActorLocation().ToString(),
		*ProjectileMovement->Velocity.ToString(),
		ProjectileMovement->InitialSpeed);
}

void AMonsterProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning,
		TEXT("OnProjectileHit: OtherActor=%s, OtherComp=%s, ImpactPoint=%s, Owner=%s"),
		*GetNameSafe(OtherActor),
		*GetNameSafe(OtherComp),
		*Hit.ImpactPoint.ToString(),
		*GetNameSafe(GetOwner()));

	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		Destroy();
		return;
	}

	// 몬스터 근접 공격(ProcessAttackOverlap)과 동일하게 플레이어만 피해 대상으로 처리
	if (OtherActor->Implements<UPlayerInterface>() &&
		OtherActor->Implements<UStatComponentUserInterface>())
	{
		if (UMonsterComponent* Source = SourceMonsterComponent.Get())
		{
			Source->ApplyAttackDamage(OtherActor, AttackMultiplier);
		}
	}

	Destroy();
}
