#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/PoolableInterface.h"
#include "Interface/EnemyInterface.h"
#include "MonsterCharacterBase.generated.h"

class UMonsterComponent;
class UStatComponent;

UCLASS()
class PROJECTWITHER_API AMonsterCharacterBase : 
    public ACharacter, 
    public IStatComponentUserInterface,
    public IPoolableInterface,
    public IEnemyInterface
{
    GENERATED_BODY()

public:
    AMonsterCharacterBase();
    virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual UStatComponent* GetStatComponent_Implementation() const override;
    virtual void OnSpawnFromPool_Implementation() override;
    virtual void OnReturnToPool_Implementation() override;

    UFUNCTION(BlueprintPure, Category = "Monster")
    UMonsterComponent* GetMonsterComponent() const { return MonsterComponent; }

    // Keep the actor-level Blueprint API; shared behavior lives in the component.
    UFUNCTION(BlueprintCallable, Category = "Monster")
    void SetMonsterState(EMonsterState NewState);
    UFUNCTION(BlueprintCallable, Category = "Monster")
    void SetTarget(AActor* NewTarget);
    UFUNCTION(BlueprintCallable, Category = "Monster")
    void ClearTarget();
    UFUNCTION(BlueprintCallable, Category = "Monster")
    AActor* GetTargetActor();
    UFUNCTION(BlueprintCallable, Category = "Monster")
    float GetDistanceToTarget();
    UFUNCTION(BlueprintCallable, Category = "Monster|Drop")
    void CalculateDrops();
    UFUNCTION(BlueprintCallable, Category = "Monster|Drop")
    void DropItems();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    TObjectPtr<UStatComponent> StatComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    TObjectPtr<UMonsterComponent> MonsterComponent;
};
