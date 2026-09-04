// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BlacksmithNPC.h"
#include "Component/InteractionComponent.h"
#include "Blueprint/UserWidget.h"

ABlacksmithNPC::ABlacksmithNPC()
{
	NPCName = FText::FromString(TEXT("대장장이"));
}

void ABlacksmithNPC::HandleInteraction(AActor* Interactor)
{
	if (!IsValid(Interactor)) { return; }

	UInteractionComponent* Interaction = Interactor->FindComponentByClass<UInteractionComponent>();

	if (!IsValid(Interaction)) { return; }

	if (Interaction->OpenInteractionUI(this, BlacksmithWidgetClass))
	{
		UE_LOG(LogTemp, Log, TEXT("대장장이 UI 열기 성공"));
	}
}
