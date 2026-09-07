#include "Component/MonsterWorldHealthBarComponent.h"

#include "Component/MonsterComponent.h"
#include "Component/StatComponent.h"
#include "Interface/StatComponentUserInterface.h"
#include "Widget/MonsterWorldHealthBarWidget.h"

#include "GameFramework/Actor.h"
#include "TimerManager.h"

UMonsterWorldHealthBarComponent::UMonsterWorldHealthBarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(180.0f, 24.0f));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetVisibility(false);
}

void UMonsterWorldHealthBarComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	if (OwnerActor->Implements<UStatComponentUserInterface>())
	{
		StatComponent = IStatComponentUserInterface::Execute_GetStatComponent(OwnerActor);
	}
	if (!IsValid(StatComponent))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[%s] MonsterWorldHealthBar: StatComponent was not found."),
			*GetNameSafe(OwnerActor));
	}

	MonsterComponent = OwnerActor->FindComponentByClass<UMonsterComponent>();

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

	UE_LOG(LogTemp, Warning,
		TEXT("[%s] MonsterWorldHealthBar: Widget Class is empty or is not derived from MonsterWorldHealthBarWidget. Current widget: %s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(UserWidget));
}

void UMonsterWorldHealthBarComponent::ShowTemporarily()
{
	SetHiddenInGame(false);
	SetVisibility(true);

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
	SetVisibility(false);
	SetHiddenInGame(true);
}
