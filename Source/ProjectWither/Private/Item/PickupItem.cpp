// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PickupItem.h"
#include "DataAsset/ItemDataAsset.h"
#include "Component/InventoryComponent.h"
#include "Item/ItemInstance.h"

#include "Components/SphereComponent.h"
#include "Components/MeshComponent.h"
#include "NiagaraComponent.h"

// Sets default values
APickupItem::APickupItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RootCollision"));
	SphereCollision->InitSphereRadius(100.0f);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(SphereCollision);

	ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMeshComponent->SetupAttachment(SphereCollision);
	ItemMeshComponent->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFX"));
	NiagaraComponent->SetupAttachment(SphereCollision);
}

void APickupItem::InitializePickup(FItemInstance InItemData)
{
	ItemInstance = InItemData;

	if (ItemMeshComponent && ItemInstance.ItemData)
	{
		ItemMeshComponent->SetStaticMesh(ItemInstance.ItemData->GetItemMesh());
	}
}

void APickupItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ItemInstance.ItemData)
	{
		InitializePickup(ItemInstance);
	}
}

// Called when the game starts or when spawned
void APickupItem::BeginPlay()
{
	Super::BeginPlay();
	
	ElapsedTime = 0.0f;

	FTimerHandle PickupDelayHandle;
	GetWorld()->GetTimerManager().SetTimer(
		PickupDelayHandle,
		[this]()
		{
			UE_LOG(LogTemp, Log, TEXT("픽업을 획득할 수 있습니다."));
			OnActorBeginOverlap.AddDynamic(this, &APickupItem::OnBeginOverlap);
		},
		PickupDelayTime,
		false
	);
}

// Called every frame
void APickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIdle)
	{
		OnUpdateUpdownSpin(DeltaTime);
	}
}

void APickupItem::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!IsValid(OtherActor) || OtherActor->FindComponentByClass<UInventoryComponent>() == nullptr)
	{
		return;
	}
	OnPickup(OtherActor);
}

void APickupItem::OnPickup(AActor* InTarget)
{
	if (GetWorldTimerManager().IsTimerActive(PickupEffectTimerHandle)) return;	// 타이머가 이미 작동중이면 종료(중복실행 방지)

	UE_LOG(LogTemp, Log, TEXT("[APickupBase] : %s(이)가 %s를 획득했습니다."),
		InTarget ? *InTarget->GetName() : TEXT("알 수 없는 대상"), *this->GetName());
	bIdle = false;

	TargetActor = InTarget;

	// 커브 에셋이 준비되어 있고 메시 컴포넌트가 있으면 연출 시작, 없으면 즉시 획득 처리
	if (IsPickupEffectAssetReady() && GetMesh())
	{
		// 더 이상의 오버랩이 발생하지 않게 하기
		SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickupStartLocation = GetMesh()->GetComponentLocation();
		PickupElapsedTime = 0.0f;

		GetWorldTimerManager().SetTimer(
			PickupEffectTimerHandle,
			this,
			&APickupItem::OnUpdatePickupEffect,
			TimerInterval,
			true
		);
	}
	else
	{
		OnFinishPickupEffect();
	}
}

void APickupItem::OnUpdatePickupEffect()
{
	if (!TargetActor.IsValid() || !GetMesh())	// 타겟이 없거나 메시가 없으면 즉시 획득 처리
	{
		OnFinishPickupEffect();
		return;
	}

	PickupElapsedTime += TimerInterval;
	float Div = FMath::Max(PickupEffectDuration, 0.001f);
	float Progress = PickupElapsedTime / Div;

	float DistanceAlpha = PickupAlpha->GetFloatValue(Progress);
	FVector Goal = TargetActor.Get()->GetActorLocation();
	FVector NewLocation = FMath::Lerp(PickupStartLocation, Goal, DistanceAlpha);

	float HeightOffset = PickupHeight->GetFloatValue(Progress) * PickupEffecHeight;
	NewLocation.Z += HeightOffset;
	GetMesh()->SetWorldLocation(NewLocation);

	float Scale = PickupScale->GetFloatValue(Progress);
	GetMesh()->SetRelativeScale3D(FVector(Scale));

	if (Progress >= 1.0f)
	{
		OnFinishPickupEffect();
	}
}

void APickupItem::OnFinishPickupEffect()
{
	GetWorldTimerManager().ClearTimer(PickupEffectTimerHandle);

	AActor* Target = TargetActor.Get();

	UInventoryComponent* Inventory = IsValid(Target)
		? Target->FindComponentByClass<UInventoryComponent>()
		: nullptr;

	if (IsValid(Inventory) &&
		IsValid(ItemInstance.ItemData.Get()) &&
		ItemInstance.Quantity > 0)
	{
		const int32 AddedQuantity = Inventory->AddItem(
			ItemInstance.ItemData.Get(),
			ItemInstance.Quantity
		);

		ItemInstance.Quantity -= AddedQuantity;

		if (ItemInstance.Quantity <= 0)
		{
			Destroy();
			return;
		}
	}

	TargetActor.Reset();
	PickupElapsedTime = 0.0f;
	bIdle = true;

	if (UMeshComponent* PickupMesh = GetMesh())
	{
		PickupMesh->SetRelativeLocation(MeshBaseLocation);
		PickupMesh->SetRelativeScale3D(FVector::OneVector);
	}

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void APickupItem::OnUpdateUpdownSpin(float InDeltaTime)
{
	if (!IsCurveAssetReady()) return;

	ElapsedTime += InDeltaTime;

	if (UMeshComponent* PickupMesh = GetMesh())
	{
		float Div = FMath::Max(UpDownDuration, 0.001f);
		float Progress = FMath::Fmod(ElapsedTime / Div, 1.0f);
		FVector NewMeshLocation = MeshBaseLocation;
		NewMeshLocation.Z += UpDownCurve->GetFloatValue(Progress) * UpDownHeight;

		PickupMesh->SetRelativeLocation(NewMeshLocation);

		float NewAngle = SpinCurve->GetFloatValue(Progress) * 360.0f;
		PickupMesh->SetRelativeRotation(FRotator(0.0f, NewAngle, 0.0f));
	}
}

UMeshComponent* APickupItem::GetMesh() const
{
	return ItemMeshComponent;
}

bool APickupItem::IsCurveAssetReady() const
{
	return UpDownCurve != nullptr && SpinCurve != nullptr;
}

bool APickupItem::IsPickupEffectAssetReady() const
{
	return PickupAlpha != nullptr && PickupHeight != nullptr && PickupScale != nullptr;
}