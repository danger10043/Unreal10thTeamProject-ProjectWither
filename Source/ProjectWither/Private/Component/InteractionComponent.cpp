// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InteractionComponent.h"

#include "Player/PlayerCharacter.h"
#include "Component/CombatComponent.h"
#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Interface/InteractableInterface.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UInteractionComponent::TryInteract()
{
	// F를 누른 순간 다시 검사한다.
	// 이전 프레임의 대상만 믿으면 시선을 돌린 뒤에도 실행될 수 있다.
	AActor* Target = FindInteractionTarget();

	CurrentTarget = Target;

	if (!IsValid(Target)) { return; }

	IInteractableInterface::Execute_Interact(Target, OwnerPlayer.Get());

}

bool UInteractionComponent::HasInteractionTarget() const
{
	return CurrentTarget.IsValid();
}

AActor* UInteractionComponent::GetInteractionTarget() const
{
	return CurrentTarget.Get();
}


void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerPlayer = Cast<APlayerCharacter>(GetOwner());
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 감지만 반복하고 상호작용은 실행하지 않는다.
	CurrentTarget = FindInteractionTarget();
}

bool UInteractionComponent::CanPlayerInteract() const
{
	if (!IsValid(OwnerPlayer) || !OwnerPlayer->IsLocallyControlled()) { return false; }

	if (!OwnerPlayer->CanMove() || OwnerPlayer->IsInventoryOpen()) { return false; }

	const UStatComponent* Stat = OwnerPlayer->FindComponentByClass<UStatComponent>();

	const UCombatComponent* Combat = OwnerPlayer->FindComponentByClass<UCombatComponent>();

	if (!IsValid(Stat) || Stat->IsHealthZero() || !IsValid(Combat)) { return false; }

	// 공격, 구르기, 가드, 피격, 사망 중에는 불가능
	return Combat->GetActionState() == EPlayerActionState::None;
}

AActor* UInteractionComponent::FindInteractionTarget() const
{
	if (!CanPlayerInteract() || !GetWorld()) { return nullptr; }

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPlayer->GetController());

	if (!IsValid(PlayerController)) { return nullptr; }

	// 실제 플레이어 시점의 위치와 방향
	FVector ViewLocation;
	FRotator ViewRotation;

	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams QueryParams;

	// 플레이어 자신을 감지하지 않도록 제외
	QueryParams.AddIgnoredActor(OwnerPlayer.Get());

	// 손에 든 무기가 감지선을 막지 않도록 제외
	const UWeaponComponent* Weapon = OwnerPlayer->FindComponentByClass<UWeaponComponent>();

	if (IsValid(Weapon))
	{
		if (AActor* WeaponActor = Weapon->GetWeaponActor())
		{
			QueryParams.AddIgnoredActor(WeaponActor);
		}
	}

	FHitResult Hit;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	if (!bHit) { return nullptr; }

	AActor* HitActor = Hit.GetActor();

	if (!IsValid(HitActor)) { return nullptr; }

	if (!HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return nullptr;
	}

	// NPC 쪽에서 플레이어와의 거리 등을 검사한다.
	if (!IInteractableInterface::Execute_CanInteract(HitActor, OwnerPlayer.Get()))
	{
		return nullptr;
	}

	return HitActor;
}

