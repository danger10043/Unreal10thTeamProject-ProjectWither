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

bool ARangedWeaponActorBase::Fire_Implementation(const FGunFireContext& FireContext)
{
	return true;
}

USceneComponent* ARangedWeaponActorBase::GetMuzzleComponent() const
{
	return Muzzle;
}


