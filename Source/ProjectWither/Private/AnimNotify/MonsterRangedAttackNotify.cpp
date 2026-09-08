// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/MonsterRangedAttackNotify.h"
#include "Component/MonsterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UMonsterRangedAttackNotify::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	UMonsterComponent* MonsterComponent =
		Owner->FindComponentByClass<UMonsterComponent>();

	if (IsValid(MonsterComponent))
	{
		MonsterComponent->FireProjectileAtTarget();
	}
}
