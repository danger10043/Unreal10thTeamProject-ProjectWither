#include "AnimNotify/AttackCheckpointAnimNotify.h"

#include "Component/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatComponentUserInterface.h"

void UAttackCheckpointAnimNotify::Notify(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = IsValid(MeshComp) ? MeshComp->GetOwner() : nullptr;
	if (!IsValid(Owner) ||
		!Owner->GetClass()->ImplementsInterface(
			UCombatComponentUserInterface::StaticClass()
		))
	{
		return;
	}

	UCombatComponent* Combat =
		ICombatComponentUserInterface::Execute_GetCombatComponent(Owner);

	if (IsValid(Combat))
	{
		Combat->ReachAttackCheckpoint(SectionName);
	}
}
