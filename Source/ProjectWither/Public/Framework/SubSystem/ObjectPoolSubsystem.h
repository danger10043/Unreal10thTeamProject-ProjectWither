// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CommonHeader/ObjectPoolEnums.h"
#include "ObjectPoolSubsystem.generated.h"

class UObjectPoolDataAsset;

using FOrderNode = TDoubleLinkedList<TObjectPtr<AActor>>::TDoubleLinkedListNode;

// 오브젝트 풀 하나를 나타낼 구조체
USTRUCT()
struct FObjectPool
{
    GENERATED_BODY()

    FObjectPool()
        : ActiveOrderList(MakeShared<TDoubleLinkedList<TObjectPtr<AActor>>>()),
        ActiveNodeMap(MakeShared<TMap<TObjectPtr<AActor>, FOrderNode*>>())
    {
    }

    // 사용 대기 중인 Actor
    UPROPERTY(Transient)
    TArray<TObjectPtr<AActor>> ReadyActors;

    // 현재 사용 중인 Actor
    UPROPERTY(Transient)
    TSet<TObjectPtr<AActor>> ActiveActors;

    // 가장 오래된 활성 Actor를 찾기 위한 순서 목록
    TSharedPtr<TDoubleLinkedList<TObjectPtr<AActor>>> ActiveOrderList;

    // Actor별 활성 순서 노드
    TSharedPtr<TMap<TObjectPtr<AActor>, FOrderNode*>> ActiveNodeMap;

    UPROPERTY(Transient)
    int32 InitialSize = 0;

    UPROPERTY(Transient)
    int32 MaxSize = 0;

    UPROPERTY(Transient)
    EObjectPoolPolicy MaxPolicy = EObjectPoolPolicy::Grow;
};

UCLASS()
class PROJECTWITHER_API UObjectPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable)
    bool RegisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset, bool bWarmup = false);

    UFUNCTION(BlueprintCallable)
    bool UnregisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset);

    UFUNCTION(BlueprintCallable)
    void Warmup(TSubclassOf<AActor> InClass);

    UFUNCTION(BlueprintCallable)
    void WarmupAll();

    UFUNCTION(BlueprintCallable)
    void ClearPool(TSubclassOf<AActor> InClass);

    UFUNCTION(BlueprintCallable)
    void ClearAllPools();

    UFUNCTION(BlueprintCallable)
    AActor* Spawn(TSubclassOf<AActor> InClassType, const FTransform& InTransform);

    template<typename T>
    T* Spawn(TSubclassOf<T> InClassType, const FTransform& InTransform)
    {
        return Cast<T>(Spawn(TSubclassOf<AActor>(InClassType), InTransform));
    }

    template<typename T>
    T* Spawn(const FTransform& InTransform)
    {
        return Cast<T>(Spawn(T::StaticClass(), InTransform));
    }

    UFUNCTION(BlueprintCallable)
    bool ReturnPool(AActor* InActor);

protected:
    AActor* CreateNewObject(TSubclassOf<AActor> InClassType, const FTransform& InTransform);

    AActor* GetReadyActor(FObjectPool* InPool);

    void RemoveInvalidActors(FObjectPool& Pool);

protected:
    UPROPERTY(Transient)
    TMap<TSubclassOf<AActor>, FObjectPool> ObjectPools;
};