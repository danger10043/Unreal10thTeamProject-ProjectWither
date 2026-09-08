#include "Component/MonsterWorldHealthBarComponent.h"

#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"
#include "Widget/MonsterWorldHealthBarWidget.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"

UMonsterWorldHealthBarComponent::UMonsterWorldHealthBarComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(180.0f, 24.0f));
	SetTwoSided(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetVisibility(false);
}

void UMonsterWorldHealthBarComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	APlayerCameraManager* CameraManager = IsValid(PlayerController)
		? PlayerController->PlayerCameraManager
		: nullptr;
	if (!IsValid(CameraManager))
	{
		return;
	}

	const FVector ToCamera = CameraManager->GetCameraLocation() - GetComponentLocation();
	if (!ToCamera.IsNearlyZero())
	{
		FRotator FacingRotation = ToCamera.Rotation();
		FacingRotation.Yaw += CameraFacingYawOffset;
		SetWorldRotation(FacingRotation);
	}
}

void UMonsterWorldHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();
	// Existing Blueprint instances may have serialized the previous Screen value.
	SetWidgetSpace(EWidgetSpace::World);
	SetTwoSided(true);

	OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	// Screen-space widget components need an explicit local player to be
	// registered in that player's screen layer reliably.
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			SetOwnerPlayer(PlayerController->GetLocalPlayer());
		}
	}

	if (OwnerActor->Implements<UStatComponentUserInterface>())
	{
		StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor);
	}
	MonsterComponent = OwnerActor->FindComponentByClass<UMonsterComponent>();
	OwnerActor->OnTakeAnyDamage.AddUniqueDynamic(
		this, &UMonsterWorldHealthBarComponent::HandleOwnerDamaged);

	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.AddUniqueDynamic(
			this, &UMonsterWorldHealthBarComponent::HandleHealthChanged);
		RefreshHealth(StatComponent->GetCurrentHealth(), StatComponent->GetMaxHealth());
	}

	if (IsValid(MonsterComponent))
	{
		MonsterComponent->OnMonsterDied.AddUniqueDynamic(
			this, &UMonsterWorldHealthBarComponent::HandleMonsterDied);
	}

	HideHealthBar();
}

void UMonsterWorldHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(OwnerActor))
	{
		OwnerActor->OnTakeAnyDamage.RemoveDynamic(
			this, &UMonsterWorldHealthBarComponent::HandleOwnerDamaged);
	}

	if (IsValid(StatComponent))
	{
		StatComponent->OnHealthChanged.RemoveDynamic(
			this, &UMonsterWorldHealthBarComponent::HandleHealthChanged);
	}

	if (IsValid(MonsterComponent))
	{
		MonsterComponent->OnMonsterDied.RemoveDynamic(
			this, &UMonsterWorldHealthBarComponent::HandleMonsterDied);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UMonsterWorldHealthBarComponent::HandleOwnerDamaged(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser)
{
	if (Damage > 0.0f && IsValid(StatComponent) &&
		(!IsValid(MonsterComponent) || !MonsterComponent->IsDead()))
	{
		ShowTemporarily();
	}
}

void UMonsterWorldHealthBarComponent::HandleHealthChanged(
	float CurrentHealth, float MaxHealth, float ChangedAmount)
{
	RefreshHealth(CurrentHealth, MaxHealth);

	if (ChangedAmount < 0.0f && CurrentHealth > 0.0f)
	{
		ShowTemporarily();
	}
}

void UMonsterWorldHealthBarComponent::HandleMonsterDied()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}

	HideHealthBar();
}

void UMonsterWorldHealthBarComponent::RefreshHealth(float CurrentHealth, float MaxHealth)
{
	InitWidget();

	UUserWidget* UserWidget = GetUserWidgetObject();
	if (UMonsterWorldHealthBarWidget* HealthWidget = Cast<UMonsterWorldHealthBarWidget>(UserWidget))
	{
		HealthWidget->SetHealth(CurrentHealth, MaxHealth);
		return;
	}

}

void UMonsterWorldHealthBarComponent::ShowTemporarily()
{
	if (!GetOwnerPlayer())
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				SetOwnerPlayer(PlayerController->GetLocalPlayer());
			}
		}
	}

	InitWidget();
	SetHiddenInGame(false);
	SetVisibility(true, true);
	SetComponentTickEnabled(true);

	if (UUserWidget* UserWidget = GetUserWidgetObject())
	{
		UserWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	MarkRenderStateDirty();

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(HideTimerHandle);

	if (VisibleDurationAfterDamage <= 0.0f)
	{
		HideHealthBar();
		return;
	}

	TimerManager.SetTimer(
		HideTimerHandle,
		this,
		&UMonsterWorldHealthBarComponent::HideHealthBar,
		VisibleDurationAfterDamage,
		false);
}

void UMonsterWorldHealthBarComponent::HideHealthBar()
{
	SetComponentTickEnabled(false);
	SetVisibility(false, true);
	SetHiddenInGame(true);
}
