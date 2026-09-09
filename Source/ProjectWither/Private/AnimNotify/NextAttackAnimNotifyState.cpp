#include "AnimNotify/NextAttackAnimNotifyState.h"

#include "Component/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatComponentUserInterface.h"

namespace
{
	UCombatComponent* FindComboCombatComponent(USkeletalMeshComponent* MeshComp)
	{
		AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;

		if (!IsValid(Owner) ||
			!Owner->GetClass()->ImplementsInterface(
				UCombatComponentUserInterface::StaticClass()
			))
		{
			return nullptr;
		}

		return ICombatComponentUserInterface::Execute_GetCombatComponent(Owner);
	}
}

void UNextAttackAnimNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UCombatComponent* Combat = FindComboCombatComponent(MeshComp))
	{
		Combat->BeginNextAttackWindow(SectionName);
	}
}

void UNextAttackAnimNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	if (UCombatComponent* Combat = FindComboCombatComponent(MeshComp))
	{
		Combat->EndNextAttackWindow(SectionName);
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
