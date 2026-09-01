// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MonsterAttackHitWindow.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Monster Attack Hit Window"))
class PROJECTWITHER_API UMonsterAttackHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	

public:
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    // 애니메이션 에디터 미리보기에서는 실행하지 않음
    virtual bool ShouldFireInEditor() override
    {
        return false;
    }

protected:
    UPROPERTY(EditAnywhere, Category = "Attack")
    FName HitboxName = TEXT("Mouth");
};
