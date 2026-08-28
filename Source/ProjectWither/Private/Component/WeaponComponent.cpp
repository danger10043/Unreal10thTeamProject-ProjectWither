#include "Component/WeaponComponent.h"

#include "Component/StatComponent.h"
#include "DataAsset/WeaponDataAsset.h"
#include "GameFramework/Actor.h"
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

	// TODO: InventoryComponent 참조 획득하기
}

void UWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyWeaponActor();

	Super::EndPlay(EndPlayReason);
}

bool UWeaponComponent::EquipWeapon(UWeaponDataAsset* WeaponData)
{
	if (!IsValid(WeaponData))
	{
		return false;
	}

	const EWeaponType WeaponType = WeaponData->GetWeaponType();

	if (WeaponType != EWeaponType::Sword &&
		WeaponType != EWeaponType::Gun)
	{
		return false;
	}

	// TODO: InventoryComponent 연동 후 구현.
	//
	// 1. WeaponData에 해당하는 보유 무기 인스턴스를 조회
	// 2. 기존 장착 무기의 변경 상태를 인벤토리에 반영
	// 3. 새 무기 Actor를 생성하고 장착 소켓에 부착
	// 4. 성공한 경우에만 CurrentWeapon과 WeaponActor를 교체
	// 5. 이전 무기 Actor를 제거
	//
	// 장착 실패 시 기존 장착 상태는 유지해야 함

	return false;
}

void UWeaponComponent::UnequipWeapon()
{
	if (GetCurrentWeapon())
	{
		// TODO: 강화 수치와 장탄수를 인벤토리에 반영한 후 해제
		// 아직 저장 경로가 없으므로 장착 데이터 버리지 않기!
		return;
	}

	DestroyWeaponActor();
	CurrentWeapon = FItemInstance();
}

bool UWeaponComponent::SwapWeapon()
{
	// TODO: 인벤토리의 검/총 장착 슬롯 조회 후 구현
	// 반대 종류의 무기를 찾지 못하면 기존 무기를 유지
	// 현재 무기를 먼저 해제하지 않고 EquipWeapon()으로 교체하기
	return false;
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
	if (!IsGunEquipped() || GetCurrentAmmo() <= 0)
	{
		return false;
	}

	if (!IsValid(StatComponent) || StatComponent->IsHealthZero())
	{
		return false;
	}

	// TODO: 투사체 또는 히트스캔 발사 구현
	//
	// 발사 실패 시 탄약은 유지
	// 아직 실제 발사가 없으므로 탄약을 차감하지 않고 실패를 반환
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

	// TODO: InventoryComponent 연동 후 구현.
	//
	// WeaponData->GetAmmoType()에 맞는 예비 탄약을
	// RequiredAmmo 이하로 실제 차감합니다.
	// 실제로 차감한 수량만 CurrentWeapon.CurrentAmmo에 더합니다.
	// 실제 장전량이 1 이상일 때만 true를 반환합니다.
	(void)RequiredAmmo;
	return false;
}

int32 UWeaponComponent::GetCurrentAmmo() const
{
	return IsGunEquipped() ? FMath::Max(0, CurrentWeapon.CurrentAmmo) : 0;
}

void UWeaponComponent::DestroyWeaponActor()
{
	if (IsValid(WeaponActor))
	{
		WeaponActor->Destroy();
	}

	WeaponActor = nullptr;
}