#include "Component/WeaponComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/CombatComponent.h"
#include "Component/StatComponent.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/Weapon/RangedWeaponActorBase.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
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
	SyncCurrentWeaponToEquipment();
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

	AActor* OwnerActor = GetOwner();
	UEquipmentComponent* EquipmentComponent =
		IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UEquipmentComponent>()
		: nullptr;

	if (IsValid(EquipmentComponent))
	{
		return EquipmentComponent->EquipItemFromInventorySlot(NewWeaponSlot);
	}

	if (!EquipWeaponInstance(NewWeaponInstance)) return false;

	CurrentWeaponSlot = NewWeaponSlot;
	return true;
}

bool UWeaponComponent::EquipWeaponInstance(const FItemInstance& WeaponInstance)
{
	if (WeaponInstance.Quantity <= 0 || !IsValid(WeaponInstance.ItemData.Get())) return false;

	UWeaponDataAsset* WeaponData = Cast<UWeaponDataAsset>(WeaponInstance.ItemData.Get());

	if (!IsValid(WeaponData)) return false;

	const EWeaponType WeaponType = WeaponData->GetWeaponType();

	if (WeaponType != EWeaponType::Sword && WeaponType != EWeaponType::Gun) return false;

	if (GetCurrentWeaponData() == WeaponData && IsValid(WeaponActor))
	{
		CurrentWeapon = WeaponInstance;
		CurrentWeaponSlot = INDEX_NONE;
		SyncCurrentWeaponToEquipment();
		OnWeaponChanged.Broadcast();
		return true;
	}

	AActor* NewWeaponActor = SpawnWeaponActor(WeaponData);
	if (!IsValid(NewWeaponActor)) return false;

	SyncCurrentWeaponToEquipment();

	if (GetCurrentWeapon() && !SaveCurrentWeaponToInventory())
	{
		NewWeaponActor->Destroy();
		return false;
	}

	DestroyWeaponActor();
	CurrentWeapon = WeaponInstance;
	CurrentWeaponSlot = INDEX_NONE;
	WeaponActor = NewWeaponActor;

	SyncCurrentWeaponToEquipment();
	OnWeaponChanged.Broadcast();

	return true;
}

void UWeaponComponent::UnequipWeapon()
{
	SyncCurrentWeaponToEquipment();

	if (GetCurrentWeapon() && !SaveCurrentWeaponToInventory())
	{
		return;
	}

	DestroyWeaponActor();
	CurrentWeapon = FItemInstance();
	CurrentWeaponSlot = INDEX_NONE;

	OnWeaponChanged.Broadcast();
}

bool UWeaponComponent::SwapWeapon()
{
	AActor* OwnerActor = GetOwner();
	UEquipmentComponent* EquipmentComponent =
		IsValid(OwnerActor)
		? OwnerActor->FindComponentByClass<UEquipmentComponent>()
		: nullptr;

	if (!IsValid(EquipmentComponent)) return false;

	SyncCurrentWeaponToEquipment();

	EWeaponType TargetWeaponType = EWeaponType::Sword;
	const bool bHasCurrentWeapon = IsSwordEquipped() || IsGunEquipped();

	if (IsSwordEquipped())
	{
		TargetWeaponType = EWeaponType::Gun;
	}
	else if (IsGunEquipped())
	{
		TargetWeaponType = EWeaponType::Sword;
	}

	FItemInstance TargetWeapon =
		TargetWeaponType == EWeaponType::Gun
		? EquipmentComponent->GetEquippedGun()
		: EquipmentComponent->GetEquippedSword();

	if (!IsValid(TargetWeapon.ItemData) || TargetWeapon.Quantity <= 0)
	{
		if (bHasCurrentWeapon)
		{
			return false;
		}

		TargetWeapon =
			TargetWeaponType == EWeaponType::Gun
			? EquipmentComponent->GetEquippedSword()
			: EquipmentComponent->GetEquippedGun();
	}

	return EquipWeaponInstance(TargetWeapon);
}

FItemInstance* UWeaponComponent::GetCurrentWeapon()
{
	return IsValid(GetCurrentWeaponData()) ? &CurrentWeapon : nullptr;
}

const FItemInstance* UWeaponComponent::GetCurrentWeapon() const
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
	const AActor* CombatOwner = GetOwner();
	const UCombatComponent* Combat =
		IsValid(CombatOwner)
		? CombatOwner->FindComponentByClass<UCombatComponent>()
		: nullptr;

	if (IsValid(Combat) &&
		Combat->GetActionState() == EPlayerActionState::Reload)
	{
		return false;
	}

	if (!IsGunEquipped())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - 캐릭터가 총을 장착하고 있지 않습니다.")
		);
		return false;
	}

	if (GetCurrentAmmo() <= 0)
	{
		if (!Reload())
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("WeaponComponent::FireGun - 재장전할 수 없습니다.")
			);
		}

		return false;
	}

	if (!IsValid(WeaponActor))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - WeaponActor가 유효하지 않습니다.")
		);
		return false;
	}

	if (!IsValid(StatComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - StatComponent가 유효하지 않습니다.")
		);
		return false;
	}

	if (StatComponent->IsHealthZero())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - 현재 플레이어의 체력이 0 입니다.")
		);
		return false;
	}

	ARangedWeaponActorBase* RangedWeapon = Cast<ARangedWeaponActorBase>(WeaponActor);

	if (!IsValid(RangedWeapon))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - 장착된 총기 액터가 ARangedWeaponActorBase를 상속하지 않았습니다. 현재 액터: %s"),
			*GetNameSafe(WeaponActor)
		);
		return false;
	}

	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - OwnerActor가 유효하지 않습니다.")
		);
		return false;
	}

	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	AController* OwnerController = IsValid(OwnerPawn) ? OwnerPawn->GetController() : nullptr;

	FVector AimOrigin = FVector::ZeroVector;
	FRotator AimRotation = FRotator::ZeroRotator;

	if (IsValid(OwnerController))
	{
		OwnerController->GetPlayerViewPoint(
			AimOrigin,
			AimRotation
		);
	}
	else
	{
		OwnerActor->GetActorEyesViewPoint(
			AimOrigin,
			AimRotation
		);
	}

	const UWeaponDataAsset* WeaponData = GetCurrentWeaponData();

	if (!IsValid(WeaponData))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::FireGun - WeaponData 가 유효하지 않습니다.")
		);
		return false;
	}

	const float MinAttackPower = 
		FMath::Min(
		StatComponent->GetMinAttackPower(),
		StatComponent->GetMaxAttackPower()
		);

	const float MaxAttackPower =
		FMath::Max(
			StatComponent->GetMinAttackPower(),
			StatComponent->GetMaxAttackPower()
		);

	FGunFireContext FireContext;
	FireContext.Shooter = OwnerActor;
	FireContext.InstigatorController = OwnerController;
	FireContext.AimOrigin = AimOrigin;
	FireContext.AimDirection = AimRotation.Vector().GetSafeNormal();
	FireContext.Damage =
		FMath::Max(
			0.0f,
			FMath::FRandRange(
				MinAttackPower,
				MaxAttackPower
			) + WeaponData->GetEnhancedWeaponPower(CurrentWeapon.EnhanceLevel)
		);

	if (!RangedWeapon->Fire(FireContext))
	{
		return false;
	}

	if (!ConsumeAmmo())
	{
		return false;
	}

	OnGunFired.Broadcast();

	if (GetCurrentAmmo() <= 0)
	{
		Reload();
	}

	return true;
}

bool UWeaponComponent::ConsumeAmmo()
{
	if (!IsGunEquipped() || CurrentWeapon.CurrentAmmo <= 0)
	{
		return false;
	}

	--CurrentWeapon.CurrentAmmo;
	SyncCurrentWeaponToEquipment();
	OnWeaponChanged.Broadcast();
	return true;
}

bool UWeaponComponent::Reload()
{
	AActor* CombatOwner = GetOwner();
	UCombatComponent* Combat =
		IsValid(CombatOwner)
		? CombatOwner->FindComponentByClass<UCombatComponent>()
		: nullptr;

	if (!IsValid(Combat) || !Combat->CanReload())
	{
		return false;
	}

	const UWeaponDataAsset* WeaponData = GetCurrentWeaponData();

	if (!WeaponData)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::Reload - WeaponData 가 유효하지 않습니다.")
		);
		return false;
	}

	if (WeaponData->GetWeaponType() != EWeaponType::Gun)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("WeaponComponent::Reload - 플레이어가 총을 장착하고 있지 않습니다.")
		);
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
	SyncCurrentWeaponToEquipment();

	Combat->PlayReloadMontage();

	OnWeaponChanged.Broadcast();

	return true;
}

int32 UWeaponComponent::GetCurrentAmmo() const
{
	return IsGunEquipped() ? FMath::Max(0, CurrentWeapon.CurrentAmmo) : 0;
}

bool UWeaponComponent::RefillCurrentWeaponAmmo()
{
	const UWeaponDataAsset* WeaponData = GetCurrentWeaponData();

	if (!WeaponData || WeaponData->GetWeaponType() != EWeaponType::Gun)
	{
		return false;
	}

	const int32 MaxAmmo = FMath::Max(0, WeaponData->GetMaxAmmo());

	if (CurrentWeapon.CurrentAmmo >= MaxAmmo)
	{
		return false;
	}

	CurrentWeapon.CurrentAmmo = MaxAmmo;
	SyncCurrentWeaponToEquipment();
	OnWeaponChanged.Broadcast();

	return true;
}

bool UWeaponComponent::SaveCurrentWeaponToInventory()
{
	if (!GetCurrentWeapon())
	{
		return true;
	}
	
	if (CurrentWeaponSlot == INDEX_NONE)
	{
		return true;
	}

	if (!IsValid(InventoryComponent))
	{
		return false;
	}

	return InventoryComponent->UpdataItemAtSlot(CurrentWeaponSlot, CurrentWeapon);
}

void UWeaponComponent::SyncCurrentWeaponToEquipment()
{
	if (!GetCurrentWeapon())
	{
		return;
	}

	AActor* OwnerActor = GetOwner();

	if (!IsValid(OwnerActor))
	{
		return;
	}

	UEquipmentComponent* EquipmentComponent = OwnerActor->FindComponentByClass<UEquipmentComponent>();

	if (IsValid(EquipmentComponent))
	{
		EquipmentComponent->UpdateEquippedWeaponState(CurrentWeapon);
	}
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
