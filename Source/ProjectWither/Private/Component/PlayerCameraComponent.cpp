#include "Component/PlayerCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Component/WeaponComponent.h"
#include "Interface/WeaponComponentUserInterface.h"
#include "Player/PlayerCharacter.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
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
		break;

	case EPlayerCameraState::Zoom:
		if (!CanZoom())
		{
			return;
		}
		break;
	case EPlayerCameraState::LockOn:
		// TODO : 락온 진입 조건과 대상 선택 구현
		return;

	default:
		return;
	}

	LockonTarget = nullptr;
	CameraState = NewState;
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

	UpdateCameraPlacement(DeltaTime);
	UpdateCameraFOV(DeltaTime);
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
