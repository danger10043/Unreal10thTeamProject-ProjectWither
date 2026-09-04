#include "Component/PlayerCameraComponent.h"

#include "CollisionQueryParams.h"
#include "Component/MonsterComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Component/WeaponComponent.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Player/PlayerCharacter.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UPlayerCameraComponent::InitializeCamera(UCameraComponent* InCameraComponent, USpringArmComponent* InSpringComponent)
{
	OwnerPlayer = Cast<APlayerCharacter>(GetOwner());
	CameraComponent = nullptr;
	SpringArmComponent = nullptr;

	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::InitializeCamera - OwnerPlayer가 유효하지 않습니다.")
			);
		return;
	}

	if (!IsValid(InCameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::InitializeCamera - InCameraComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(InSpringComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::InitializeCamera - InSpringComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (InCameraComponent->GetOwner() != OwnerPlayer.Get())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::InitializeCamera - 카메라가 소유 플레이어의 컴포넌트가 아닙니다.")
		);
		return;
	}

	if (InSpringComponent->GetOwner() != OwnerPlayer.Get())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::InitializeCamera - 스프링 암이 소유 플레이어의 컴포넌트가 아닙니다.")
		);
		return;
	}

	CameraComponent = InCameraComponent;
	SpringArmComponent = InSpringComponent;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("PlayerCameraComponent::InitializeCamera - CameraComponent가 %s 로 할당되었습니다."),
		*CameraComponent->GetName()
	);
	UE_LOG(
		LogTemp,
		Log,
		TEXT("PlayerCameraComponent::InitializeCamera - SpringArmComponent가 %s 로 할당되었습니다."),
		*SpringArmComponent->GetName()
	);

	NormalFOV = CameraComponent->FieldOfView;
	CameraState = EPlayerCameraState::None;
	LockonTarget = nullptr;
	ResetLockOnLookInput();

	NormalSocketOffset = SpringArmComponent->SocketOffset;
	NormalArmLength = SpringArmComponent->TargetArmLength;

	SpringArmComponent->AddTickPrerequisiteComponent(this);

	SetComponentTickEnabled(true);
}

void UPlayerCameraComponent::HandleLookInput(const FVector2D& Input)
{
	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::HandleLookInput - OwnerPlayer가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::HandleLookInput - CameraComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(SpringArmComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::HandleLookInput - SpringArmComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!OwnerPlayer->IsLocallyControlled())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::HandleLookInput - OwnerPlayer가 로컬에서 조작되고 있지 않습니다.")
		);
		return;
	}

	if (!IsValid(OwnerPlayer->GetController()))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::HandleLookInput - 플레이어 컨트롤러가 유효하지 않습니다.")
		);
		return;
	}

	HandleLockOnLookInput(Input);

	OwnerPlayer->AddControllerYawInput(Input.X);
	OwnerPlayer->AddControllerPitchInput(Input.Y);

}

void UPlayerCameraComponent::StartZoom()
{
	ChangeCameraState(EPlayerCameraState::Zoom);
}

void UPlayerCameraComponent::StopZoom()
{
	if (CameraState != EPlayerCameraState::Zoom)
	{
		return;
	}

	ChangeCameraState(EPlayerCameraState::None);
}

void UPlayerCameraComponent::ChangeCameraState(EPlayerCameraState NewState)
{
	switch (NewState)
	{
	case EPlayerCameraState::None:
	{
		LockonTarget = nullptr;
		break;
	}

	case EPlayerCameraState::Zoom:
	{
		if (!CanZoom())
		{
			return;
		}

		LockonTarget = nullptr;
		break;
	}

	case EPlayerCameraState::LockOn:
	{
		if (CameraState != EPlayerCameraState::None)
		{
			return;
		}

		AActor* NewTarget = FindLockOnTarget();
		if (!IsValid(NewTarget))
		{
			return;
		}

		LockonTarget = NewTarget;
		break;
	}
	default:
		return;
	}

	CameraState = NewState;
}

void UPlayerCameraComponent::ToggleLockOn()
{
	if (CameraState == EPlayerCameraState::LockOn)
	{
		ClearLockOn();
		return;
	}

	if (CameraState != EPlayerCameraState::None)
	{
		return;
	}

	ChangeCameraState(EPlayerCameraState::LockOn);
}

AActor* UPlayerCameraComponent::FindLockOnTarget()
{
	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - OwnerPlayer가 유효하지 않습니다.")
		);
		return nullptr;
	}

	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - CameraComponent가 유효하지 않습니다.")
		);
		return nullptr;
	}

	if (!IsValid(SpringArmComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - SpringArmComponent가 유효하지 않습니다.")
		);
		return nullptr;
	}

	if (!OwnerPlayer->IsLocallyControlled())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - OwnerPlayer가 로컬에서 조작되고 있지 않습니다.")
		);
		return nullptr;
	}

	if (OwnerPlayer->IsInventoryOpen())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - OwnerPlayer의 인벤토리가 열려 있어 락온을 실행할 수 없습니다.")
		);
		return nullptr;
	}

	if (LockOnRange <= 0.0f)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::FindLockOnTarget - 락온 사정거리가 유효하지 않습니다.")
		);
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPlayer->GetController());

	UWorld* World = GetWorld();
	if (!IsValid(PC) || !IsValid(World)) return nullptr;

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// 카메라가 플레이어 뒤에 떨어져 있는 거리까지 Trace 길이에 포함
	const float CameraToPlayerDistance = FVector::Distance(
		ViewLocation,
		OwnerPlayer->GetActorLocation()
	);

	const FVector TraceEnd =
		ViewLocation + ViewRotation.Vector() * (LockOnRange + CameraToPlayerDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPlayer.Get());

	const UWeaponComponent* WeaponComponent =
		IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwnerPlayer.Get());

	if (IsValid(WeaponComponent))
	{
		AActor* WeaponActor = WeaponComponent->GetWeaponActor();
		if (IsValid(WeaponActor))
		{
			QueryParams.AddIgnoredActor(WeaponActor);
		}
	}

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	))
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	return IsValidLockOnTarget(HitActor) ? HitActor : nullptr;
}

void UPlayerCameraComponent::ClearLockOn()
{
	LockonTarget = nullptr;

	if (CameraState == EPlayerCameraState::LockOn)
	{
		ChangeCameraState(EPlayerCameraState::None);
	}
}

void UPlayerCameraComponent::UpdateLockOnRotation(float DeltaTime)
{
	if (CameraState != EPlayerCameraState::LockOn) return;

	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::UpdateLockOnRotation - OwnerPlayer가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::UpdateLockOnRotation - CameraComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(SpringArmComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::UpdateLockOnRotation - SpringArmComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValidLockOnTarget(LockonTarget.Get()))
	{
		ClearLockOn();
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPlayer->GetController());
	UWorld* World = GetWorld();

	if (!IsValid(PC) || !IsValid(World) || !OwnerPlayer->IsLocallyControlled() || PC->IsLookInputIgnored())
	{
		return;
	}

	// 마우스 조작 직후에는 자동 회전을 잠시 유예
	const double Now = World->GetTimeSeconds();
	if (LastLockOnMouseInputTime >= 0.0 && Now - LastLockOnMouseInputTime < LockOnReturnDelay)
	{
		return;
	}

	const FVector TargetLocation =
		LockonTarget->GetActorLocation() + LockOnTargetOffset;

	const FVector ToTarget = TargetLocation - CameraComponent->GetComponentLocation();

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FRotator DesiredRotation = ToTarget.Rotation();
	
	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
	if (IsValid(CameraManager))
	{
		CameraManager->LimitViewPitch(
			DesiredRotation,
			CameraManager->ViewPitchMin,
			CameraManager->ViewPitchMax
		);
	}

	FRotator NewRotation = FMath::RInterpTo(
		PC->GetControlRotation(),
		DesiredRotation,
		DeltaTime,
		LockOnRotationInterpSpeed
	);

	NewRotation.Roll = 0.0f;
	PC->SetControlRotation(NewRotation);
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::TickComponent - OwnerPlayer가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::TickComponent - CameraComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!IsValid(SpringArmComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::TickComponent - SpringArmComponent가 유효하지 않습니다.")
		);
		return;
	}

	if (!OwnerPlayer->IsLocallyControlled())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::TickComponent - OwnerPlayer가 로컬에서 조작되고 있지 않습니다.")
		);
		return;
	}

	if (CameraState == EPlayerCameraState::LockOn &&
		(OwnerPlayer->IsInventoryOpen() || !IsValidLockOnTarget(LockonTarget.Get())))
	{
		ClearLockOn();
	}

	UpdateCameraPlacement(DeltaTime);
	UpdateCameraFOV(DeltaTime);

	if (CameraState == EPlayerCameraState::LockOn)
	{
		UpdateLockOnRotation(DeltaTime);
	}

	//if (CameraState == EPlayerCameraState::LockOn && IsValidLockOnTarget(LockonTarget.Get()))
	//{
	//	UE_LOG(
	//		LogTemp,
	//		Log,
	//		TEXT("PlayerCameraComponent::TickComponent - 락온 On, 락온 대상 { %s }"),
	//		*LockonTarget->GetName()
	//	);
	//}
	//else
	//{
	//	UE_LOG(
	//		LogTemp,
	//		Log,
	//		TEXT("PlayerCameraComponent::TickComponent - 락온 Off")
	//	);
	//}
}

void UPlayerCameraComponent::UpdateCameraPlacement(float DeltaTime)
{
	const UWeaponComponent* WeaponComponent =
		IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwnerPlayer.Get());

	const bool bIsGunEquipped =
		IsValid(WeaponComponent) && WeaponComponent->IsGunEquipped();

	const FVector TargetSocketOffset =
		bIsGunEquipped ? NormalSocketOffset + RangedSocketOffset : NormalSocketOffset;

	const float TargetArmLength =
		bIsGunEquipped ? RangedArmLength : NormalArmLength;

	SpringArmComponent->SocketOffset =
		FMath::VInterpTo(
			SpringArmComponent->SocketOffset,
			TargetSocketOffset,
			DeltaTime,
			CameraPlacementInterpSpeed
		);

	SpringArmComponent->TargetArmLength =
		FMath::FInterpTo(
			SpringArmComponent->TargetArmLength,
			TargetArmLength,
			DeltaTime,
			CameraPlacementInterpSpeed
		);
}

void UPlayerCameraComponent::UpdateCameraFOV(float DeltaTime)
{
	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::UpdateCameraFOV - CameraComponent 가 유효하지 않습니다.")
		);
		return;
	}

	if (CameraState == EPlayerCameraState::Zoom && !CanZoom())
	{
		StopZoom();
	}

	const float TargetFOV =
		CameraState == EPlayerCameraState::Zoom ? FMath::Min(ZoomFOV, NormalFOV) : NormalFOV;

	CameraComponent->SetFieldOfView(
		FMath::FInterpTo(
			CameraComponent->FieldOfView,
			TargetFOV,
			DeltaTime,
			ZoomInterpSpeed
		)
	);
}

bool UPlayerCameraComponent::CanZoom() const
{
	if (!IsValid(OwnerPlayer))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::CanZoom - OwnerPlayer가 유효하지 않습니다.")
		);
		return false;
	}

	if (!IsValid(CameraComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::CanZoom - CameraComponent가 유효하지 않습니다.")
		);
		return false;
	}

	if (!IsValid(SpringArmComponent))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::CanZoom - SpringArmComponent가 유효하지 않습니다.")
		);
		return false;
	}

	if (!OwnerPlayer->IsLocallyControlled())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::CanZoom - OwnerPlayer가 로컬에서 조작되고 있지 않습니다.")
		);
		return false;
	}

	if (OwnerPlayer->IsInventoryOpen())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PlayerCameraComponent::CanZoom - OwnerPlayer의 인벤토리가 열려 있어 줌 기능을 사용할 수 없습니다.")
		);
		return false;
	}

	const UWeaponComponent* WeaponComponent =
		IWeaponComponentUserInterface::Execute_GetWeaponComponent(OwnerPlayer.Get());

	return IsValid(WeaponComponent) && WeaponComponent->IsGunEquipped();
}

bool UPlayerCameraComponent::IsValidLockOnTarget(AActor* Target) const
{
	if (!IsValid(OwnerPlayer) || !IsValid(Target) || Target == OwnerPlayer.Get())
	{
		return false;
	}

	if (Target->IsHidden() || !Target->GetActorEnableCollision())
	{
		return false;
	}

	const UMonsterComponent* MonsterComponent =
		Target->FindComponentByClass<UMonsterComponent>();

	if (!IsValid(MonsterComponent) || MonsterComponent->IsDead())
	{
		return false;
	}

	if (LockOnRange <= 0.0f)
	{
		return false;
	}

	return
		FVector::DistSquared(OwnerPlayer->GetActorLocation(), Target->GetActorLocation()) <=
		FMath::Square(LockOnRange);
}

void UPlayerCameraComponent::HandleLockOnLookInput(const FVector2D& Input)
{
	if (CameraState != EPlayerCameraState::LockOn || Input.IsNearlyZero())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(OwnerPlayer))
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPlayer->GetController());
	if (!IsValid(PC) || PC->IsLookInputIgnored())
	{
		return;
	}

	const double Now = World->GetTimeSeconds();
	LastLockOnMouseInputTime = Now;

	// 최근 입력 구간보다 오래된 기록 제거
	LockOnMouseSamples.RemoveAll(
		[Now, this](const FLockOnMouseSample& Sample)
		{
			return Now - Sample.Time > LockOnInputWindow;
		}
	);

	FLockOnMouseSample NewSample;
	NewSample.Time = Now;
	NewSample.Delta = Input;
	LockOnMouseSamples.Add(NewSample);

	FVector2D Displacement = FVector2D::ZeroVector;
	for (const FLockOnMouseSample& Sample : LockOnMouseSamples)
	{
		Displacement += Sample.Delta;
	}

	if (Displacement.SizeSquared() >= FMath::Square(LockOnBreakInputThreshold))
	{
		ClearLockOn();
	}
}

void UPlayerCameraComponent::ResetLockOnLookInput()
{
	LockOnMouseSamples.Reset();
	LastLockOnMouseInputTime = -1.0;
}
