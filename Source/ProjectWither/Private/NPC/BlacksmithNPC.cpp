// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BlacksmithNPC.h"

ABlacksmithNPC::ABlacksmithNPC()
{
	NPCName = FText::FromString(TEXT("대장장이"));
}

void ABlacksmithNPC::HandleInteraction(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("%s: %s와 상호작용 시작"), *NPCName.ToString(), *GetNameSafe(Interactor));

	// 다음 단계에서 대장장이 UI 열기 요청 연결
}
