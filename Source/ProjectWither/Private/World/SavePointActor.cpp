// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SavePointActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Engine/GameInstance.h"
#include "Framework/SubSystem/MonsterSpawnSubsystem.h"
#include "Framework/SubSystem/SavePointSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Player/PlayerCharacter.h"
#include "Widget/SavePointMenuWidget.h"

ASavePointActor::ASavePointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SavePointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SavePointMesh"));
	SavePointMesh->SetupAttachment(SceneRoot);
	SavePointMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SavePointMesh->SetCollisionObjectType(ECC_WorldStatic);
	SavePointMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SavePointMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SavePointMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	RespawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPoint"));
	RespawnPoint->SetupAttachment(SceneRoot);
	RespawnPoint->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f));
}

void ASavePointActor::BeginPlay()
{
	Super::BeginPlay();

	if (SavePointId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASavePointActor::BeginPlay - %s 의 SavePointId가 지정되지 않았습니다."), *GetName());
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USavePointSubsystem* Subsystem = GameInstance->GetSubsystem<USavePointSubsystem>())
		{
			Subsystem->RegisterSavePoint(SavePointId, DisplayName, GetRespawnTransform());
		}
	}
}

bool ASavePointActor::CanInteract_Implementation(AActor* Interactor) const
{
	if (!IsValid(Interactor) || Interactor == this) { return false; }

	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());

	return DistanceSquared <= FMath::Square(InteractionDistance);
}

void ASavePointActor::Interact_Implementation(AActor* Interactor)
{
	if (!IInteractableInterface::Execute_CanInteract(this, Interactor)) { return; }

	APlayerCharacter* Player = Cast<APlayerCharacter>(Interactor);

	if (!IsValid(Player) || SavePointId.IsNone()) { return; }

	UGameInstance* GameInstance = GetGameInstance();
	USavePointSubsystem* Subsystem = IsValid(GameInstance) ? GameInstance->GetSubsystem<USavePointSubsystem>() : nullptr;

	if (!IsValid(Subsystem)) { return; }

	const bool bWasActivated = Subsystem->IsSavePointActivated(SavePointId);

	// 상호작용할 때마다 이 지점을 현재(마지막) 세이브 포인트로 지정한다.
	Subsystem->ActivateSavePoint(SavePointId);

	if (!bWasActivated)
	{
		OnActivated();
	}

	RestPlayer(Player);
	OnRested();

	// 휴식할 때마다 레벨에 배치된 몬스터를 전부 리스폰
	if (UWorld* World = GetWorld())
	{
		if (UMonsterSpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UMonsterSpawnSubsystem>())
		{
			SpawnSubsystem->RespawnAllZones();
		}
	}

	OpenSavePointMenu(Player);
}

void ASavePointActor::RestPlayer(APlayerCharacter* Player) const
{
	if (!IsValid(Player)) { return; }

	UStatComponent* Stat = IStatComponentUserInterface::Execute_GetStatComponent(Player);

	if (IsValid(Stat))
	{
		// 체력과 스태미나를 최대치로 회복
		Stat->ResetStat();
	}

	if (UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>())
	{
		Inventory->RefillPotionsToMax();
	}

	UWeaponComponent* Weapon = IWeaponComponentUserInterface::Execute_GetWeaponComponent(Player);

	if (IsValid(Weapon))
	{
		Weapon->RefillCurrentWeaponAmmo();
	}
}

void ASavePointActor::OpenSavePointMenu(APlayerCharacter* Player)
{
	if (!IsValid(SavePointMenuClass) || !IsValid(Player) || !Player->IsLocallyControlled()) { return; }

	APlayerController* PlayerController = Cast<APlayerController>(Player->GetController());

	if (!IsValid(PlayerController)) { return; }

	if (!IsValid(SavePointMenuInstance))
	{
		SavePointMenuInstance = CreateWidget<USavePointMenuWidget>(PlayerController, SavePointMenuClass);
	}

	if (!IsValid(SavePointMenuInstance)) { return; }

	if (!SavePointMenuInstance->IsInViewport())
	{
		SavePointMenuInstance->AddToViewport();
	}

	SavePointMenuInstance->OpenAt(SavePointId);

	PlayerController->bShowMouseCursor = true;

	// 이 메뉴는 버튼 클릭으로만 조작하므로 키보드 포커스 대상을 지정할 필요가 없다.
	// SetWidgetToFocus를 쓰면 UUserWidget의 Slate 래퍼가 포커스 불가로 취급되어
	// "Attempting to focus Non-Focusable widget" 경고가 발생한다.
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}

FTransform ASavePointActor::GetRespawnTransform() const
{
	if (IsValid(RespawnPoint))
	{
		return RespawnPoint->GetComponentTransform();
	}

	return GetActorTransform();
}
