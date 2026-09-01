#include "AnimNotify/SwordAttackAnimNotifyState.h"

#include "Component/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatComponentUserInterface.h"

namespace
{
	UCombatComponent* FindCombatComponent(USkeletalMeshComponent* MeshComp)
	{
		if (!IsValid(MeshComp)) return nullptr;

		AActor* OwnerActor = MeshComp->GetOwner();

		if (!IsValid(OwnerActor) || !OwnerActor->GetClass()->ImplementsInterface(
			UCombatComponentUserInterface::StaticClass()
		))
		{
			return nullptr;
		}

		return ICombatComponentUserInterface::Execute_GetCombatComponent(OwnerActor);
	}
}

void USwordAttackAnimNotifyState::NotifyBegin(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	float TotalDuration, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UCombatComponent* CombatComponent = FindCombatComponent(MeshComp))
	{
		CombatComponent->BeginSwordDamageWindow();
	}
}

void USwordAttackAnimNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	if (UCombatComponent* CombatComponent = FindCombatComponent(MeshComp))
	{
		CombatComponent->EndSwordDamageWindow();
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
