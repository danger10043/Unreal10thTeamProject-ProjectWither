// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

    // 마우스를 움직여도 캐릭터 몸은 함께 회전하지 않습니다.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 이동 방향으로 캐릭터가 회전합니다.
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationRate, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    CameraArm = CreateDefaultSubobject<USpringArmComponent>( TEXT("CameraArm"));
    CameraArm->SetupAttachment(GetRootComponent());
    CameraArm->TargetArmLength = CameraDistance;
    CameraArm->bUsePawnControlRotation = true;

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>( TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment( CameraArm, USpringArmComponent::SocketName);
    PlayerCamera->bUsePawnControlRotation = false;

}

void APlayerCharacter::SetCanMove(bool bNewCanMove)
{
    bCanMove = bNewCanMove;

    if (!bCanMove)
    {
        StopRun();
        GetCharacterMovement()->StopMovementImmediately();
    }
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    const APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (!PlayerController || !DefaultMappingContext) { return; }
    ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

    if (!LocalPlayer) { return; }
    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

    if (InputSubsystem)
    {
        InputSubsystem->AddMappingContext( DefaultMappingContext, 0);
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput =  Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInput) { return; }
    EnhancedInput->BindAction( MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
    EnhancedInput->BindAction( LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
    EnhancedInput->BindAction( RunAction, ETriggerEvent::Started, this, &APlayerCharacter::StartRun);
    EnhancedInput->BindAction( RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRun);
    EnhancedInput->BindAction( RunAction, ETriggerEvent::Canceled, this, &APlayerCharacter::StopRun);
    EnhancedInput->BindAction( RollAction, ETriggerEvent::Started, this, &APlayerCharacter::StartRoll);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!bCanMove || !Controller) { return; }
	const FVector2D Input = Value.Get<FVector2D>();

	if (Input.IsNearlyZero()) { return; }
    const FRotator ControlRotation = Controller->GetControlRotation();
    const FRotator YawRotation( 0.0f, ControlRotation.Yaw, 0.0f);
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, Input.Y);
    AddMovementInput(RightDirection, Input.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
    if (!Controller) { return; }
    const FVector2D Input = Value.Get<FVector2D>();

    AddControllerYawInput(Input.X);
    AddControllerPitchInput(Input.Y);
}

void APlayerCharacter::StartRun()
{
    if (!bCanMove) { return; }
    bIsRunning = true;
    UpdateMovementSpeed();
}

void APlayerCharacter::StopRun()
{
    bIsRunning = false;
    UpdateMovementSpeed();
}

void APlayerCharacter::UpdateMovementSpeed()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();

    if (!Movement) { return; }
    Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
}

void APlayerCharacter::StartRoll()
{
    if (!bCanMove || bIsRolling || !RollMontage) { return; }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement || !Movement->IsMovingOnGround()) { return; }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance) { return; }

    // 입력 중이면 입력 방향으로 구릅니다.
    FVector RollDirection = GetLastMovementInputVector().GetSafeNormal2D();
    // 입력이 없으면 캐릭터가 바라보는 방향으로 구릅니다.
    if (RollDirection.IsNearlyZero())
    {
        RollDirection = GetActorForwardVector().GetSafeNormal2D();
    }

    StopRun();
    Movement->StopMovementImmediately();

    SetActorRotation(RollDirection.Rotation());

    bIsRolling = true;
    bCanMove = false;

    const float PlayedLength = PlayAnimMontage(RollMontage);

    if (PlayedLength <= 0.0f)
    {
        bIsRolling = false;
        bCanMove = true;
        return;
    }

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject( this, &APlayerCharacter::OnRollMontageEnded);

    AnimInstance->Montage_SetEndDelegate( EndDelegate, RollMontage);
}

void APlayerCharacter::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != RollMontage) { return; }

    bIsRolling = false;
    bCanMove = true;
}

