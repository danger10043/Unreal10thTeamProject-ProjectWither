#include "Component/WeaponComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/StatComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Interface/StatComponentUserInterface.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();

	if (!ensureMsgf(
		IsValid(OwnerActor),
		TEXT("WeaponComponent의 Owner가 유효하지 않습니다.")
	))
	{
		return;
	}

	if (!ensureMsgf(
		OwnerActor->GetClass()->ImplementsInterface(
			UStatComponentUserInterface::StaticClass()
		),
		TEXT("WeaponComponent의 Owner는 Stat 인터페이스를 구현해야 합니다.")
	))
	{
		return;
	}

	StatComponent =
		IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor);

	if (!ensureMsgf(
		IsValid(StatComponent),
		TEXT("WeaponComponent의 StatComponent가 유효하지 않습니다.")
	))
	{
		return;
	}

	InventoryComponent = OwnerActor->FindComponentByClass<UInventoryComponent>();

	ensureMsgf(
		IsValid(InventoryComponent),
		TEXT("WeaponComponent의 Owner에 InventoryComponent가 유효하지 않습니다.")
	);
}

void UWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveCurrentWeaponToInventory();
	DestroyWeaponActor();

	Super::EndPlay(EndPlayReason);
}

bool UWeaponComponent::EquipWeapon(UWeaponDataAsset* WeaponData)
{
	if (!IsValid(WeaponData) || !IsValid(InventoryComponent)) return false;

	const EWeaponType WeaponType = WeaponData->GetWeaponType();

	if (WeaponType != EWeaponType::Sword && WeaponType != EWeaponType::Gun) return false;

	const int32 NewWeaponSlot = InventoryComponent->FindItemSlot(WeaponData->GetItemId());

	if (NewWeaponSlot == INDEX_NONE) return false;

	if (NewWeaponSlot == CurrentWeaponSlot &&
		GetCurrentWeaponData() == WeaponData &&
		IsValid(WeaponActor))
	{
		return true;
	}

	FItemInstance NewWeaponInstance;

	if (!InventoryComponent->GetItemAtSlot(NewWeaponSlot, NewWeaponInstance)) return false;

	if (NewWeaponInstance.Quantity <= 0 || NewWeaponInstance.ItemData.Get() != WeaponData) return false;

	AActor* NewWeaponActor = SpawnWeaponActor(WeaponData);
	if (!IsValid(NewWeaponActor)) return false;

	if (GetCurrentWeapon() && !SaveCurrentWeaponToInventory())
	{
		NewWeaponActor->Destroy();
		return false;
	}

	DestroyWeaponActor();
	CurrentWeapon = NewWeaponInstance;
	CurrentWeaponSlot = NewWeaponSlot;
	WeaponActor = NewWeaponActor;

	return true;
}

void UWeaponComponent::UnequipWeapon()
{
	if (GetCurrentWeapon() && !SaveCurrentWeaponToInventory())
	{
		return;
	}

	DestroyWeaponActor();
	CurrentWeapon = FItemInstance();
	CurrentWeaponSlot = INDEX_NONE;
}

bool UWeaponComponent::SwapWeapon()
{
	if (!IsValid(InventoryComponent)) return false;

	EWeaponType TargetWeaponType = EWeaponType::Sword;

	if (IsSwordEquipped())
	{
		TargetWeaponType = EWeaponType::Gun;
	}
	else if (IsGunEquipped())
	{
		TargetWeaponType = EWeaponType::Sword;
	}
	
	const int32 TargetSlot = InventoryComponent->FindWeaponSlotByType(TargetWeaponType);
	if (TargetSlot == INDEX_NONE)
	{
		return false;
	}
	
	FItemInstance TargetWeapon;

	if (!InventoryComponent->GetItemAtSlot(TargetSlot, TargetWeapon)) return false;

	UWeaponDataAsset* TargetWeaponData = Cast<UWeaponDataAsset>(TargetWeapon.ItemData.Get());

	return IsValid(TargetWeaponData) && EquipWeapon(TargetWeaponData);
}

FItemInstance* UWeaponComponent::GetCurrentWeapon()
{
	return IsValid(GetCurrentWeaponData()) ? &CurrentWeapon : nullptr;
}

UWeaponDataAsset* UWeaponComponent::GetCurrentWeaponData() const
{
	if (CurrentWeapon.Quantity <= 0 || !IsValid(CurrentWeapon.ItemData.Get()))
	{
		return nullptr;
	}

	UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(CurrentWeapon.ItemData.Get());

	return IsValid(WeaponData) ? WeaponData : nullptr;
}

EWeaponType UWeaponComponent::GetWeaponType() const
{
	const UWeaponDataAsset* WeaponData = GetCurrentWeaponData();

	return WeaponData ? WeaponData->GetWeaponType() : EWeaponType::None;
}

bool UWeaponComponent::IsSwordEquipped() const
{
	return GetWeaponType() == EWeaponType::Sword;
}

bool UWeaponComponent::IsGunEquipped() const
{
	return GetWeaponType() == EWeaponType::Gun;
}

bool UWeaponComponent::FireGun()
{
	if (!IsGunEquipped() || GetCurrentAmmo() <= 0 || !IsValid(WeaponActor)) return false;

	if (!IsValid(StatComponent) || StatComponent->IsHealthZero()) return false;

	if (!PerformGunFire(WeaponActor)) return false;

	return ConsumeAmmo();
}

bool UWeaponComponent::PerformGunFire_Implementation(AActor* CurrentWeaponActor)
{
	return false;
}

bool UWeaponComponent::ConsumeAmmo()
{
	if (!IsGunEquipped() || CurrentWeapon.CurrentAmmo <= 0)
	{
		return false;
	}

	--CurrentWeapon.CurrentAmmo;
	return true;
}

bool UWeaponComponent::Reload()
{
	const UWeaponDataAsset* WeaponData = GetCurrentWeaponData();

	if (!WeaponData || WeaponData->GetWeaponType() != EWeaponType::Gun)
	{
		return false;
	}

	const int32 MaxAmmo = FMath::Max(0, WeaponData->GetMaxAmmo());

	if (CurrentWeapon.CurrentAmmo < 0 ||
		CurrentWeapon.CurrentAmmo >= MaxAmmo)
	{
		return false;
	}

	const int32 RequiredAmmo = MaxAmmo - CurrentWeapon.CurrentAmmo;

	if (!IsValid(InventoryComponent)) return false;
	
	const int32 LoadedAmmo = InventoryComponent->ConsumeAmmoByType(WeaponData->GetAmmoType(), RequiredAmmo);

	if (LoadedAmmo <= 0) return false;

	CurrentWeapon.CurrentAmmo = FMath::Clamp(CurrentWeapon.CurrentAmmo + LoadedAmmo, 0, MaxAmmo);

	return true;
}

int32 UWeaponComponent::GetCurrentAmmo() const
{
	return IsGunEquipped() ? FMath::Max(0, CurrentWeapon.CurrentAmmo) : 0;
}

bool UWeaponComponent::SaveCurrentWeaponToInventory()
{
	if (!GetCurrentWeapon())
	{
		return true;
	}
	
	if (!IsValid(InventoryComponent) || CurrentWeaponSlot == INDEX_NONE)
	{
		return false;
	}

	return InventoryComponent->UpdataItemAtSlot(CurrentWeaponSlot, CurrentWeapon);
}

AActor* UWeaponComponent::SpawnWeaponActor(const UWeaponDataAsset* WeaponData) const
{
	if (!IsValid(WeaponData) || !WeaponData->GetWeaponActorClass() || !GetWorld()) return nullptr;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (!IsValid(OwnerCharacter) || !IsValid(OwnerCharacter->GetMesh())) return nullptr;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewWeaponActor = GetWorld()->SpawnActor<AActor>(
		WeaponData->GetWeaponActorClass(),
		FTransform::Identity,
		SpawnParameters
	);

	if (!IsValid(NewWeaponActor)) return nullptr;

	const bool bAttached = NewWeaponActor->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponData->GetAttachSocketName()
	);

	if (!bAttached)
	{
		NewWeaponActor->Destroy();
		return nullptr;
	}

	TArray<UCapsuleComponent*> CapsuleComponents;
	NewWeaponActor->GetComponents<UCapsuleComponent>(CapsuleComponents);

	for (UCapsuleComponent* CapsuleComponent : CapsuleComponents)
	{
		if (!IsValid(CapsuleComponent) || !CapsuleComponent->ComponentHasTag(TEXT("SwordHitCollision")))
		{
			continue;
		}

		CapsuleComponent->SetGenerateOverlapEvents(true);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return NewWeaponActor;
}

void UWeaponComponent::DestroyWeaponActor()
{
	if (IsValid(WeaponActor))
	{
		WeaponActor->Destroy();
	}

	WeaponActor = nullptr;
}