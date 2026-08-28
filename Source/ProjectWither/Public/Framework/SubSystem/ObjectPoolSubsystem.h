// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CommonHeader/ObjectPoolEnums.h"
#include "ObjectPoolSubsystem.generated.h"

class UObjectPoolDataAsset;

// 활성 순서 목록에서 사용하는 노드 타입
using FOrderNode = TDoubleLinkedList<TObjectPtr<AActor>>::TDoubleLinkedListNode;

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

    // 가장 오래된 활성 액터를 찾기 위한 순서 목록, 액터의 GC 참조는 ActiveActors가 담당
    TSharedPtr<TDoubleLinkedList<TObjectPtr<AActor>>> ActiveOrderList;

    // 활성 액터와 순서 목록 노드를 연결하는 내부 인덱스
    TSharedPtr<TMap<TObjectPtr<AActor>, FOrderNode*>> ActiveNodeMap;

    // 월드 시작 시 미리 생성할 액터 수
    UPROPERTY(Transient)
    int32 InitialSize = 0;

    // Grow 이외의 정책에서 풀에 유지할 최대 액터 수
    UPROPERTY(Transient)
    int32 MaxSize = 32;

    // MaxSize에 도달했을 때 새로운 획득 요청을 처리하는 정책
    UPROPERTY(Transient)
    EObjectPoolPolicy MaxPolicy = EObjectPoolPolicy::Grow;
};

// 월드별로 액터 풀을 등록하고 생성, 재사용, 반환하는 Subsystem
UCLASS()
class PROJECTWITHER_API UObjectPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 프로젝트 설정에 등록된 풀 정보를 읽고 액터 생성은 월드 BeginPlay 이후에 수행
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 등록된 모든 풀을 월드 시작 시 예열(WarmUp)
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // 월드가 종료될 때 풀에서 관리하는 액터와 런타임 상태를 정리(ClearAllPools)
    virtual void Deinitialize() override;

    // 데이터 에셋의 설정으로 풀을 등록하고 bWarmup이 true이면 등록 직후 예열
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    bool RegisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset, bool bWarmup = false);

    // 데이터 에셋이 가리키는 액터 클래스의 풀을 등록 해제하고 정리
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    bool UnregisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset);

    // 지정한 액터 클래스의 풀을 InitialSize까지 예열
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void Warmup(TSubclassOf<AActor> InClass);

    // 등록된 모든 풀을 예열
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void WarmupAll();

    // 지정한 액터 클래스의 풀과 풀에서 관리하는 액터를 모두 제거
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void ClearPool(TSubclassOf<AActor> InClass);

    // 등록된 모든 풀과 풀에서 관리하는 액터를 제거
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    void ClearAllPools();

    // 지정한 클래스의 액터를 풀에서 획득하고 전달받은 Transform으로 활성화
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
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

    // 사용 중인 액터를 해당 클래스의 풀로 반환
    UFUNCTION(BlueprintCallable, Category = "Object Pool")
    bool ReturnPool(AActor* InActor);

protected:
    // 월드에 새로운 풀 액터를 생성
    AActor* CreateNewObject(TSubclassOf<AActor> InClassType, const FTransform& InTransform);

    // 대기 목록에서 재사용할 수 있는 유효한 액터를 하나 꺼내기
    AActor* GetReadyActor(FObjectPool* InPool);

    // 외부에서 파괴된 액터를 풀의 모든 런타임 컨테이너에서 제거
    void RemoveInvalidActors(FObjectPool& Pool);

protected:
    // 액터 클래스별 풀의 런타임 상태, 월드가 종료되면 함께 정리됨
    UPROPERTY(Transient)
    TMap<TSubclassOf<AActor>, FObjectPool> ObjectPools;
};
