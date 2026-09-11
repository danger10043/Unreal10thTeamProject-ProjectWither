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

bool ARangedWeaponActorBase::Fire(const FGunFireContext& FireContext)
{
	if (!CanFire()) return false;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return false;

	const double FireTime = static_cast<double>(World->GetTimeSeconds());

	bExecutingFire = true;
	const bool bFired = ExecuteFire(FireContext);
	bExecutingFire = false;

	if (!bFired) return false;

	NextAllowedFireTime = FireTime + static_cast<double>(FireInterval);
	return true;
}

bool ARangedWeaponActorBase::ExecuteFire_Implementation(
	const FGunFireContext& FireContext)
{
	return false;
}

USceneComponent* ARangedWeaponActorBase::GetMuzzleComponent() const
{
	return Muzzle;
}


