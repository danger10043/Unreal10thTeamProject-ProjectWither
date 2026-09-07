#include "Widget/LockOnWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Component/PlayerCameraComponent.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void ULockOnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bLockOnActive = false;
	bDeactivating = false;
	TrackedTarget.Reset();

	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));

	const float SafeSize = FMath::Max(1.0f, MarkerSize);
	SetDesiredSizeInViewport(FVector2D(SafeSize, SafeSize));

	if (IsValid(RootBox))
	{
		RootBox->SetWidthOverride(SafeSize);
		RootBox->SetHeightOverride(SafeSize);
		RootBox->SetVisibility(ESlateVisibility::Hidden);
	}

	const FVector2D ScreenSize =
		UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(
			GetOwningPlayer()
		).GetLocalSize();

	SetPositionInViewport(ScreenSize * 0.5f, false);
}

void ULockOnWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	APawn* OwningPawn = GetOwningPlayerPawn();
	UPlayerCameraComponent* CameraComponent = IsValid(OwningPawn)
		? OwningPawn->FindComponentByClass<UPlayerCameraComponent>()
		: nullptr;

	AActor* CurrentTarget = IsValid(CameraComponent)
		? CameraComponent->GetLockOnTarget()
		: nullptr;

	if (IsValid(CurrentTarget))
	{
		if (!bLockOnActive || TrackedTarget.Get() != CurrentTarget)
		{
			ActivateLockOn(CurrentTarget);
		}
	}
	else if (bLockOnActive)
	{
		DeactivateLockOn();
	}

	if (bLockOnActive || bDeactivating)
	{
		UpdateScreenPosition();
	}
}

void ULockOnWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == LockonDeactivate.Get() && bDeactivating && !bLockOnActive)
	{
		bDeactivating = false;
		TrackedTarget.Reset();

		if (IsValid(RootBox))
		{
			RootBox->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ULockOnWidget::ActivateLockOn(AActor* Target)
{
	bDeactivating = false;
	bLockOnActive = true;
	TrackedTarget = Target;
	LastTargetLocation = Target->GetActorLocation() + TargetOffset;

	StopAllAnimations();

	if (IsValid(LockonImage))
	{
		LockonImage->SetRenderOpacity(1.0f);
		LockonImage->SetRenderTransform(FWidgetTransform());
	}

	UpdateScreenPosition();

	if (IsValid(LockonActivate))
	{
		PlayAnimation(LockonActivate, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
}

void ULockOnWidget::DeactivateLockOn()
{
	bLockOnActive = false;
	bDeactivating = true;

	if (IsValid(LockonActivate))
	{
		StopAnimation(LockonActivate);
	}

	if (IsValid(LockonDeactivate))
	{
		PlayAnimation(LockonDeactivate, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
	else
	{
		bDeactivating = false;
		TrackedTarget.Reset();

		if (IsValid(RootBox))
		{
			RootBox->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ULockOnWidget::UpdateScreenPosition()
{
	if (!IsValid(RootBox))
	{
		return;
	}

	if (AActor* Target = TrackedTarget.Get())
	{
		LastTargetLocation = Target->GetActorLocation() + TargetOffset;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!IsValid(PlayerController))
	{
		RootBox->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	FVector2D ScreenPosition;
	const bool bProjected =
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PlayerController,
			LastTargetLocation,
			ScreenPosition,
			true
		);

	const FVector2D ScreenSize =
		UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(
			PlayerController
		).GetLocalSize();

	const bool bInsideScreen =
		bProjected &&
		ScreenPosition.X >= 0.0 &&
		ScreenPosition.Y >= 0.0 &&
		ScreenPosition.X <= ScreenSize.X &&
		ScreenPosition.Y <= ScreenSize.Y;

	if (!bInsideScreen)
	{
		SetPositionInViewport(ScreenSize * 0.5f, false);
		RootBox->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	
	SetPositionInViewport(ScreenPosition, false);
	RootBox->SetVisibility(ESlateVisibility::HitTestInvisible);
}

