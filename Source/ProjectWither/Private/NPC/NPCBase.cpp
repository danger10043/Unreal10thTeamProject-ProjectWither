// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("NPCMesh"));
	NPCMesh->SetupAttachment(SceneRoot);

	// 지금 단계에서는 메시 대신 별도 박스로 감지
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(SceneRoot);

	// 액터 원점을 발밑으로 사용하는 임시 크기
	InteractionCollision->SetBoxExtent(FVector(30.0f, 30.0f, 90.0f));
	InteractionCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));

	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 이후 Visibility Trace로 NPC를 감지할 예정
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 플레이어가 NPC 몸을 통과하지 않도록 설정
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// 이번 방식은 Overlap 이벤트를 사용하지 않음
	InteractionCollision->SetGenerateOverlapEvents(false);

	NPCName = FText::FromString(TEXT("NPC"));
}

bool ANPCBase::CanInteract_Implementation(AActor* Interactor) const
{
	if (!bInteractionEnabled || !IsValid(Interactor)) { return false; }

	if (Interactor == this) { return false; }

	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());

	return DistanceSquared <= FMath::Square(InteractionDistance);
}

void ANPCBase::Interact_Implementation(AActor* Interactor)
{
	// 안내가 표시된 이후 상황이 달라질 수 있으므로 다시 검사
	if (!IInteractableInterface::Execute_CanInteract(this, Interactor)) { return; }

	HandleInteraction(Interactor);
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();
}

void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANPCBase::HandleInteraction(AActor* Interaction)
{
	// 실제 작업은 자식에서 구현
}

