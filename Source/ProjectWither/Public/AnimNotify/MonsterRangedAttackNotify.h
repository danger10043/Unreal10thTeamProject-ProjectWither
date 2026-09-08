// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MonsterRangedAttackNotify.generated.h"

/**
 * 원거리 공격 몽타주에서 투사체를 발사할 프레임에 배치하는 노티파이
 */
UCLASS(meta = (DisplayName = "Monster Ranged Attack"))
class PROJECTWITHER_API UMonsterRangedAttackNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	// 애니메이션 에디터 미리보기에서는 실행하지 않음
	virtual bool ShouldFireInEditor() override
	{
		return false;
	}
};
