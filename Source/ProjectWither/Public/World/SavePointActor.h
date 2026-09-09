// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "SavePointActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class APlayerCharacter;
class USavePointMenuWidget;

/*
* 오픈월드에 배치하는 세이브 포인트(휴식처) 액터.
* - 처음 상호작용하면 활성화되어 부활 지점으로 지정된다.
* - 활성화된 상태에서 다시 상호작용하면 휴식(체력/스태미나 회복, 물약/총알 충전)을 수행하고
*   다른 활성화된 세이브 포인트로 이동할 수 있는 메뉴를 연다.
*/
UCLASS()
class PROJECTWITHER_API ASavePointActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASavePointActor();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;

	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	FORCEINLINE FName GetSavePointId() const { return SavePointId; }

protected:
	virtual void BeginPlay() override;

	// 이 세이브 포인트를 처음 활성화했을 때 호출 (연출용)
	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint")
	void OnActivated();

	// 휴식(체력/물약/총알 충전)을 수행할 때마다 호출 (연출용)
	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint")
	void OnRested();

private:
	void RestPlayer(APlayerCharacter* Player) const;

	void OpenSavePointMenu(APlayerCharacter* Player);

	FTransform GetRespawnTransform() const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SavePoint|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SavePoint|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SavePointMesh;

	// 실제 부활/이동 지점으로 사용할 위치. 조형물 발밑이 아닌 앞쪽 공간 등으로 따로 배치할 수 있다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SavePoint|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RespawnPoint;

	// 레벨 내에서 겹치지 않아야 하는 고유 식별자. 저장/휴식/이동은 모두 이 Id로 구분한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SavePoint", meta = (AllowPrivateAccess = "true"))
	FName SavePointId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SavePoint", meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SavePoint", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", Units = "cm"))
	float InteractionDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SavePoint|UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<USavePointMenuWidget> SavePointMenuClass;

	UPROPERTY(Transient)
	TObjectPtr<USavePointMenuWidget> SavePointMenuInstance;
};
