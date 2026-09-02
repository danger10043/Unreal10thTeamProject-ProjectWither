#include "DataAsset/WeaponDataAsset.h"

UWeaponDataAsset::UWeaponDataAsset()
{
	ItemType = EItemType::Weapon;   // 무기 데이터 에셋은 항상 아이템 타입을 Weapon으로 고정
	MaxStack = 1;                   // 무기는 기본적으로 중첩되지 않는 장비 아이템으로 취급
}

EWeaponType UWeaponDataAsset::GetWeaponType() const
{
	return WeaponType;
}

EWeaponGunType UWeaponDataAsset::GetWeaponGunType() const
{
	return WeaponGunType;
}

EAmmoType UWeaponDataAsset::GetAmmoType() const
{
	return AmmoType;
}

int32 UWeaponDataAsset::GetMaxAmmo() const
{
	return MaxAmmo;
}

float UWeaponDataAsset::GetWeaponPower() const
{
	return WeaponPower;
}

TSubclassOf<AActor> UWeaponDataAsset::GetWeaponActorClass() const
{
	return WeaponActorClass;
}

FName UWeaponDataAsset::GetAttachSocketName() const
{
	return AttachSocketName;
}

UNiagaraSystem* UWeaponDataAsset::GetWeaponHitEffect() const
{
	return WeaponHitEffect;
}

UNiagaraSystem* UWeaponDataAsset::GetWeaponTrailEffect() const
{
	return WeaponTrailEffect;
}

UNiagaraSystem* UWeaponDataAsset::GetWeaponParryingEffect() const
{
	return WeaponParryingEffect;
}

UNiagaraSystem* UWeaponDataAsset::GetWeaponFireEffect() const
{
	return WeaponFireEffect;
}
