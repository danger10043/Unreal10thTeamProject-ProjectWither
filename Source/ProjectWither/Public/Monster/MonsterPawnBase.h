#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "Interface/StatComponentUserInterface.h"
#include "Interface/PoolableInterface.h"
#include "Interface/EnemyInterface.h"
#include "MonsterPawnBase.generated.h"

class UMonsterComponent;
class UStatComponent;

UCLASS()
class PROJECTWITHER_API AMonsterPawnBase :
    public APawn,
    public IStatComponentUserInterface,
    public IPoolableInterface,
    public IEnemyInterface
{
    GENERATED_BODY()

public:
    AMonsterPawnBase();
    virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual UStatComponent* GetStatComponent_Implementation() const override;
    virtual void OnSpawnFromPool_Implementation() override;
    virtual void OnReturnToPool_Implementation() override;
    virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime = 0.0f) override;

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

public:
    virtual void Tick(float DeltaTime) override;

protected:
    // Disable for flying monsters. Enabled by default to preserve existing ground Pawns.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bSnapToFloor = true;
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationInterpSpeed = 5.f;
    UPROPERTY(EditAnywhere, Category = "Movement")
    float FloorTraceDistance = 1000.f;
    UPROPERTY(EditAnywhere, Category = "Movement")
    float HeightAboveFloor = 0.f;
    UPROPERTY(EditAnywhere, Category = "Movement")
    float TraceOffsetRadius = 800.f;
    // 목표 높이로 오르내리는 초당 속도(유닛/초). AI 이동이 아래로 끌어당기는 힘보다
    // 커야 높이가 밀리지 않고, 격차가 커도(바위/몬스터 위 통과 등) 이 속도로만
    // 부드럽게 이동하므로 갑자기 튀지 않음
    UPROPERTY(EditAnywhere, Category = "Movement")
    float HeightCorrectionSpeed = 10.f;
    void SnapToFloor(float DeltaTime);
};
