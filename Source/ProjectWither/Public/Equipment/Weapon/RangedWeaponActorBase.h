#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RangedWeaponActorBase.generated.h"

class AController;
class USceneComponent;

USTRUCT(BlueprintType)
struct PROJECTWITHER_API FGunFireContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Gun")
	TObjectPtr<AActor> Shooter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Gun")
	TObjectPtr<AController> InstigatorController = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Gun")
	FVector AimOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Gun")
	FVector AimDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Gun")
	float Damage = 0.0f;
};

UCLASS(Blueprintable)
class PROJECTWITHER_API ARangedWeaponActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARangedWeaponActorBase();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Gun")
	bool Fire(const FGunFireContext& FireContext, bool& bOutConsumeAmmo);

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun")
	bool CanFire() const;

	void SetFireInterval(float InFireInterval);

	UFUNCTION(BlueprintPure, Category = "Weapon|Gun")
	USceneComponent* GetMuzzleComponent() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon|Gun")
	bool ExecuteFire(
		const FGunFireContext& FireContext,
		bool& bOutConsumeAmmo
	);

	virtual bool ExecuteFire_Implementation(
		const FGunFireContext& FireContext,
		bool& bOutConsumeAmmo
	);

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Weapon|Gun",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Weapon|Gun",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Weapon|Gun",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0",
			Units = "cm"
		)
	)
	float HitscanRange = 10000.0f;

private:
	float FireInterval = 0.2f;
	double NextAllowedFireTime = 0.0;
	bool bExecutingFire = false;
};
