// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UMonsterComponent;

UCLASS(Blueprintable)
class PROJECTWITHER_API AMonsterProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMonsterProjectile();

	// 발사한 몬스터의 MonsterComponent와 발사 방향을 받아 투사체 속도 및 데미지 배율을 설정
	void InitializeProjectile(
		UMonsterComponent* InSourceMonsterComponent,
		const FVector& LaunchDirection,
		float InAttackMultiplier = 1.0f);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float ProjectileLifetime = 5.0f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMonsterComponent> SourceMonsterComponent;

	float AttackMultiplier = 1.0f;
};
