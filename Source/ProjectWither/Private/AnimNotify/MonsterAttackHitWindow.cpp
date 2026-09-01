// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/MonsterAttackHitWindow.h"
#include "Component/MonsterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UMonsterAttackHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(
        MeshComp, Animation, TotalDuration, EventReference);

    if (!IsValid(MeshComp)) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner)) return;

    UMonsterComponent* MonsterComponent =
        Owner->FindComponentByClass<UMonsterComponent>();

    if (IsValid(MonsterComponent))
    {
        MonsterComponent->BeginAttackHitWindow(HitboxName);
    }
}

void UMonsterAttackHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!IsValid(MeshComp)) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner)) return;

    UMonsterComponent* MonsterComponent =
        Owner->FindComponentByClass<UMonsterComponent>();

    if (IsValid(MonsterComponent))
    {
        MonsterComponent->EndAttackHitWindow(HitboxName);
    }
}
