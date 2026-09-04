#include "Component/PlayerCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/PlayerCharacter.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

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
