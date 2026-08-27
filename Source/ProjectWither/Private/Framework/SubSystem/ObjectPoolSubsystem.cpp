// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/SubSystem/ObjectPoolSubsystem.h"
#include "Config/ObjectPoolSettings.h"
#include "Interface/PoolableInterface.h"
#include "DataAsset/ObjectPoolDataAsset.h"

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 프로젝트 세팅에서 데이터 읽어오기
	const UObjectPoolSettings* Settings = GetDefault<UObjectPoolSettings>();
	if (!Settings) return;

	for (const TSoftObjectPtr<UObjectPoolDataAsset>& DataAsset : Settings->PoolDataAssets)
	{
		if (!DataAsset.IsNull())
		{
			UObjectPoolDataAsset* LoadedDataAsset = DataAsset.LoadSynchronous();

			if (!IsValid(LoadedDataAsset) || LoadedDataAsset->ActorClass.IsNull())
			{
				continue;
			}

			UClass* LoadedClass = LoadedDataAsset->ActorClass.LoadSynchronous();

			if (!IsValid(LoadedClass))
			{
				continue;
			}

			FObjectPool& Pool = ObjectPools.FindOrAdd(LoadedClass);

			Pool.InitialSize = LoadedDataAsset->InitialSize;
			Pool.MaxSize = LoadedDataAsset->MaxSize;
			Pool.MaxPolicy = LoadedDataAsset->MaxPolicy;
		}
	}
}

void UObjectPoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	WarmupAll();

}

void UObjectPoolSubsystem::Deinitialize()
{
	ClearAllPools();
	Super::Deinitialize();
}

bool UObjectPoolSubsystem::RegisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset, bool bWarmup)
{
	if (!InDataAsset || InDataAsset->ActorClass.IsNull()) return false;

	TSubclassOf<AActor> LoadedActorClass = InDataAsset->ActorClass.LoadSynchronous();

	if (!LoadedActorClass)
	{
		return false;
	}

	ClearPool(LoadedActorClass);	// 이미 있으면 정리

	FObjectPool& Pool = ObjectPools.Add(LoadedActorClass);	// 새로 추가
	Pool.InitialSize = InDataAsset->InitialSize;			// 초기값 세팅
	Pool.MaxSize = InDataAsset->MaxSize;
	Pool.MaxPolicy = InDataAsset->MaxPolicy;

	if (bWarmup)
	{
		Warmup(LoadedActorClass);	// 웜업 요청있으면 웜업도 하기
	}

	return true;
}

bool UObjectPoolSubsystem::UnregisterPoolDataAsset(const UObjectPoolDataAsset* InDataAsset)
{
	if (!InDataAsset || InDataAsset->ActorClass.IsNull()) return false;

	TSubclassOf<AActor> LoadedActorClass = InDataAsset->ActorClass.LoadSynchronous();

	if (!LoadedActorClass || !ObjectPools.Contains(LoadedActorClass))
	{
		return false;
	}

	ClearPool(LoadedActorClass);

	return true;
}

void UObjectPoolSubsystem::Warmup(TSubclassOf<AActor> InClass)
{
	if (FObjectPool* Pool = ObjectPools.Find(InClass))
	{
		const int32 CurrentCount =
			Pool->ReadyActors.Num() + Pool->ActiveActors.Num();

		const int32 TargetCount =
			FMath::Min(Pool->InitialSize, Pool->MaxSize);

		const int32 CreateCount =
			FMath::Max(0, TargetCount - CurrentCount);

		FTransform Init(FVector::DownVector * 10000.0f);
		for (int i = 0; i < CreateCount; i++)
		{
			// 액터 스폰해서 바로 ReadyActors에 넣기			
			AActor* Spawned = CreateNewObject(InClass, Init);
			UE_LOG(LogTemp, Log, TEXT("Warmup : %s"), Spawned ? *Spawned->GetName() : TEXT("None"));

			if (Spawned)
			{
				if (Spawned->GetClass()->ImplementsInterface(UPoolableInterface::StaticClass()))
				{
					IPoolableInterface::Execute_OnReturnToPool(Spawned);
				}

				Spawned->SetActorHiddenInGame(true);
				Spawned->SetActorTickEnabled(false);
				Spawned->SetActorEnableCollision(false);

				Pool->ReadyActors.Add(Spawned);
			}
		}
	}
}

void UObjectPoolSubsystem::WarmupAll()
{
	for (auto& [Key, _] : ObjectPools)
	{
		Warmup(Key);
	}
}

void UObjectPoolSubsystem::ClearPool(TSubclassOf<AActor> InClass)
{
	if (FObjectPool* Pool = ObjectPools.Find(InClass))
	{
		for (AActor* Actor : Pool->ReadyActors)
		{
			if (IsValid(Actor)) Actor->Destroy();
		}
		Pool->ReadyActors.Empty();
		for (AActor* Actor : Pool->ActiveActors)
		{
			if (IsValid(Actor)) Actor->Destroy();
		}
		Pool->ActiveActors.Empty();
		Pool->ActiveOrderList->Empty();
		Pool->ActiveNodeMap->Empty();
		ObjectPools.Remove(InClass);
	}
}

void UObjectPoolSubsystem::ClearAllPools()
{
	for (auto& [Key, Pool] : ObjectPools)
	{
		for (AActor* Actor : Pool.ReadyActors)
		{
			if (IsValid(Actor)) Actor->Destroy();
		}
		Pool.ReadyActors.Empty();
		for (AActor* Actor : Pool.ActiveActors)
		{
			if (IsValid(Actor)) Actor->Destroy();
		}
		Pool.ActiveActors.Empty();
		Pool.ActiveOrderList->Empty();
		Pool.ActiveNodeMap->Empty();
	}
	ObjectPools.Empty();
}

AActor* UObjectPoolSubsystem::Spawn(TSubclassOf<AActor> InClassType, const FTransform& InTransform)
{
	if (!InClassType) return nullptr;

	FObjectPool* Pool = ObjectPools.Find(InClassType);
	if (!Pool) return nullptr;

	RemoveInvalidActors(*Pool);

	AActor* Spawned = nullptr;

	// ReadyActors에 쓸 수 있는게 있으면 사용
	Spawned = GetReadyActor(Pool);
	if (Spawned)
	{
		Spawned->SetActorTransform(InTransform);
		UE_LOG(LogTemp, Log, TEXT("Spawn(Reuse) : %s"), Spawned ? *Spawned->GetName() : TEXT("None"));
	}
	else
	{
		// ReadyActors에서 쓸 수 있는 것이 없으면 새로 만들기
		const uint32 TotalCount = Pool->ActiveActors.Num() + Pool->ReadyActors.Num();
		const bool bMax = TotalCount >= static_cast<uint32>(Pool->MaxSize);

		if (!bMax)
		{
			Spawned = CreateNewObject(InClassType, InTransform);
			UE_LOG(LogTemp, Log, TEXT("Spawn(New) : %s"), Spawned ? *Spawned->GetName() : TEXT("None"));
		}
		else
		{
			switch (Pool->MaxPolicy)
			{
			case EObjectPoolPolicy::DoNotSpawn:
				return nullptr;	// 그냥 스폰 안함
			case EObjectPoolPolicy::Grow:
				Spawned = CreateNewObject(InClassType, InTransform);
				UE_LOG(LogTemp, Log, TEXT("Spawn(New) : %s"), Spawned ? *Spawned->GetName() : TEXT("None"));
				break;
			case EObjectPoolPolicy::ReuseOldest:
				if (FOrderNode* HeadNode = Pool->ActiveOrderList->GetHead())
				{
					AActor* OldestActor = HeadNode->GetValue();
					if (!ReturnPool(OldestActor))
					{
						return nullptr;
					}

					Spawned = GetReadyActor(Pool);

					if (!IsValid(Spawned))
					{
						return nullptr;
					}

					Spawned->SetActorTransform(InTransform);

					UE_LOG(LogTemp, Log, TEXT("Spawn(Oldest) : %s"), Spawned ? *Spawned->GetName() : TEXT("None"));
				}
				break;
			}
		}
	}

	if (Spawned)
	{
		Spawned->SetActorHiddenInGame(false);
		Spawned->SetActorTickEnabled(true);
		Spawned->SetActorEnableCollision(true);

		Pool->ActiveActors.Add(Spawned);

		FOrderNode* NewNode = new FOrderNode(Spawned);
		Pool->ActiveOrderList->AddTail(NewNode);
		Pool->ActiveNodeMap->Add(Spawned, NewNode);

		if (Spawned->GetClass()->ImplementsInterface(
			UPoolableInterface::StaticClass()))
		{
			IPoolableInterface::Execute_OnSpawnFromPool(Spawned);
		}
	}

	return Spawned;
}

bool UObjectPoolSubsystem::ReturnPool(AActor* InActor)
{
	if (!IsValid(InActor)) return false;

	TSubclassOf<AActor> ActorClass = InActor->GetClass();
	FObjectPool* Pool = ObjectPools.Find(ActorClass);

	if (!Pool || !Pool->ActiveActors.Contains(InActor)) return false;

	UE_LOG(LogTemp, Log, TEXT("Return : %s"), InActor ? *InActor->GetName() : TEXT("None"));

	Pool->ActiveActors.Remove(InActor);

	if (FOrderNode** FoundNode = Pool->ActiveNodeMap->Find(InActor))
	{
		Pool->ActiveOrderList->RemoveNode(*FoundNode, true);
		Pool->ActiveNodeMap->Remove(InActor);
	}

	InActor->SetActorEnableCollision(false);
	InActor->SetActorTickEnabled(false);
	InActor->SetActorHiddenInGame(true);

	if (InActor->GetClass()->ImplementsInterface(UPoolableInterface::StaticClass()))
	{
		IPoolableInterface::Execute_OnReturnToPool(InActor);
	}

	if (IsValid(InActor))
	{
		Pool->ReadyActors.Add(InActor);
	}

	return true;
}

AActor* UObjectPoolSubsystem::CreateNewObject(TSubclassOf<AActor> InClassType, const FTransform& InTransform)
{
	if (!GetWorld()) return nullptr;

	FActorSpawnParameters SpawnParam;
	SpawnParam.Owner = nullptr;
	SpawnParam.ObjectFlags = RF_Transient;
	SpawnParam.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Spawned = GetWorld()->SpawnActor<AActor>(InClassType, InTransform, SpawnParam);
#if WITH_EDITOR
	if (Spawned)
	{
		Spawned->SetFolderPath(FName("Pool"));
	}
#endif
	return Spawned;
}

AActor* UObjectPoolSubsystem::GetReadyActor(FObjectPool* InPool)
{
	if (!InPool) return nullptr;

	AActor* ReadyActor = nullptr;
	while (InPool->ReadyActors.Num() > 0)
	{
		AActor* Candidate = InPool->ReadyActors.Pop();
		if (IsValid(Candidate))
		{
			ReadyActor = Candidate;
			break;
		}
	}
	return ReadyActor;
}

void UObjectPoolSubsystem::RemoveInvalidActors(FObjectPool& Pool)
{
	Pool.ReadyActors.RemoveAll(
		[](const TObjectPtr<AActor>& Actor)
		{
			return !IsValid(Actor);
		});

	for (auto It = Pool.ActiveActors.CreateIterator(); It; ++It)
	{
		AActor* Actor = *It;

		if (IsValid(Actor))
		{
			continue;
		}

		if (FOrderNode** Node =
			Pool.ActiveNodeMap->Find(Actor))
		{
			Pool.ActiveOrderList->RemoveNode(*Node, true);
			Pool.ActiveNodeMap->Remove(Actor);
		}

		It.RemoveCurrent();
	}
}
