// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCBase.h"
#include "StatUpgradeNPC.generated.h"

class UStatUpgradeWindowWidget;
class UAnimMontage;

UCLASS()
class PROJECTWITHER_API AStatUpgradeNPC : public ANPCBase
{
	GENERATED_BODY()

public:
	AStatUpgradeNPC();

	// 상호작용 종료(창 닫기) 시 위젯에서 호출: 인사 몽타주를 End 구간으로 넘겨 마무리
	void EndGreetingMontage();

protected:
	virtual void HandleInteraction(AActor* Interactor) override;

private:
	// 상호작용을 시작한 플레이어 쪽을 바라보도록 회전
	void FacePlayer(const AActor* Interactor);

	// 인사 몽타주를 Start 구간부터 재생 (이후 Loop 구간에서 대기)
	void PlayGreetingMontage();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UStatUpgradeWindowWidget> StatUpgradeWindowClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> GreetingMontage;

	// GreetingMontage 내 시작 구간 이름 (몽타주 에디터의 섹션 이름과 일치해야 함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	FName GreetingStartSection = TEXT("Start");

	// GreetingMontage 내 종료 구간 이름 (몽타주 에디터의 섹션 이름과 일치해야 함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Stat Upgrade", meta = (AllowPrivateAccess = "true"))
	FName GreetingEndSection = TEXT("End");

	UPROPERTY(Transient)
	TObjectPtr<UStatUpgradeWindowWidget> StatUpgradeWindowInstance;
};
