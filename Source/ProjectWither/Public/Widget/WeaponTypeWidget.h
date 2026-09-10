#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommonHeader/WeaponTypeEnums.h"
#include "WeaponTypeWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class PROJECTWITHER_API UWeaponTypeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Weapon")
	void SetWeaponType(EWeaponType WeaponType);

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponTypeImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Weapon")
	TObjectPtr<UTexture2D> WeaponNone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Weapon")
	TObjectPtr<UTexture2D> WeaponSword;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Weapon")
	TObjectPtr<UTexture2D> WeaponGun;

private:
	void RefreshWeaponImage();

	EWeaponType CurrentWeaponType = EWeaponType::None;
};