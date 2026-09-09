#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttackCheckpointAnimNotify.generated.h"


UCLASS()
class PROJECTWITHER_API UAttackCheckpointAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;

	UPROPERTY(EditAnywhere, Category = "Combo")
	FName SectionName = TEXT("Attack1");
};
