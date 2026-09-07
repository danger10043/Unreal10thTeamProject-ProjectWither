// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "NPCBase.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UBoxComponent;

UCLASS()
class PROJECTWITHER_API ANPCBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ANPCBase();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetNPCName() const { return NPCName; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 공통 검사를 통과한 뒤 실행할 NPC별 기능
	virtual void HandleInteraction(AActor* Interaction);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	TObjectPtr<USkeletalMeshComponent> NPCMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NPC|Interaction", meta = (ClampMin = "0.0", Units = "cm"))
	float InteractionDistance = 250.0f;

};
