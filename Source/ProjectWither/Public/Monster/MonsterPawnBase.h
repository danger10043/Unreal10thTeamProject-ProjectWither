#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CommonHeader/MonsterStateEnums.h"
#include "Interface/StatComponentUserInterface.h"
#include "MonsterPawnBase.generated.h"

class UMonsterComponent;
class UStatComponent;

UCLASS()
class PROJECTWITHER_API AMonsterPawnBase : public APawn, public IStatComponentUserInterface
{
    GENERATED_BODY()

public:
    AMonsterPawnBase();
    virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual UStatComponent* GetStatComponent_Implementation() const override;

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
    void SnapToFloor(float DeltaTime);
};
