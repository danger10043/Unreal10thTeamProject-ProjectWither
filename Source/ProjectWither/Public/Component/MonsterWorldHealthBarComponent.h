#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "MonsterWorldHealthBarComponent.generated.h"

class UMonsterComponent;
class UMonsterWorldHealthBarWidget;
class UStatComponent;

/**
 * Screen-space overhead health bar for normal monsters.
 * Add this component only to normal-monster Blueprints, not bosses.
 */
UCLASS(ClassGroup = (Monster), meta = (BlueprintSpawnableComponent))
class PROJECTWITHER_API UMonsterWorldHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UMonsterWorldHealthBarComponent();
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float ChangedAmount);

	UFUNCTION()
	void HandleOwnerDamaged(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser);

	UFUNCTION()
	void HandleMonsterDied();

	void RefreshHealth(float CurrentHealth, float MaxHealth);
	void ShowTemporarily();
	void HideHealthBar();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Monster|UI", meta = (ClampMin = "0.0", Units = "s"))
	float VisibleDurationAfterDamage = 3.0f;

	// Change to 180 if the Widget Blueprint appears mirrored/backwards.
	UPROPERTY(EditAnywhere, Category = "Monster|UI")
	float CameraFacingYawOffset = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UStatComponent> StatComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMonsterComponent> MonsterComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OwnerActor = nullptr;

	FTimerHandle HideTimerHandle;
};
