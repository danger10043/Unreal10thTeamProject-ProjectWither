#include "Equipment/Weapon/RangedWeaponActorBase.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

ARangedWeaponActorBase::ARangedWeaponActorBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(SceneRoot);
}

void ARangedWeaponActorBase::SetFireInterval(float InFireInterval)
{
	FireInterval = FMath::Max(0.01f, InFireInterval);
}

bool ARangedWeaponActorBase::CanFire() const
{
	const UWorld* World = GetWorld();

	return IsValid(World) &&
		!bExecutingFire &&
		static_cast<double>(World->GetTimeSeconds()) >= NextAllowedFireTime;
}

bool ARangedWeaponActorBase::Fire(
	const FGunFireContext& FireContext,
	bool& bOutConsumeAmmo)
{
	bOutConsumeAmmo = false;

	if (!CanFire()) return false;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return false;

	const double FireTime = static_cast<double>(World->GetTimeSeconds());

	bool bConsumeAmmo = true;

	bExecutingFire = true;
	const bool bFired = ExecuteFire(FireContext, bConsumeAmmo);
	bExecutingFire = false;

	if (!bFired) return false;

	bOutConsumeAmmo = bConsumeAmmo;
	NextAllowedFireTime = FireTime + static_cast<double>(FireInterval);
	return true;
}

bool ARangedWeaponActorBase::ExecuteFire_Implementation(
	const FGunFireContext& FireContext,
	bool& bOutConsumeAmmo)
{
	bOutConsumeAmmo = true;
	return false;
}

USceneComponent* ARangedWeaponActorBase::GetMuzzleComponent() const
{
	return Muzzle;
}


