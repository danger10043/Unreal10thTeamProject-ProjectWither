// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Component/CombatComponent.h"

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

    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(GetRootComponent());
    CameraArm->TargetArmLength = CameraDistance;
    CameraArm->bUsePawnControlRotation = true;

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment( CameraArm, USpringArmComponent::SocketName);
    PlayerCamera->bUsePawnControlRotation = false;

    StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
    WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
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

UStatComponent* APlayerCharacter::GetStatComponent_Implementation() const
{
    return StatComponent;
}

UWeaponComponent* APlayerCharacter::GetWeaponComponent_Implementation() const
{
    return WeaponComponent;
}

UCombatComponent* APlayerCharacter::GetCombatComponent_Implementation() const
{
    return CombatComponent;
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

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(RunStaminaTimerHandle);

    Super::EndPlay(EndPlayReason);
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
    if (!bCanMove || !IsValid(StatComponent)) return;


    if (StatComponent->GetCurrentStamina() <= 0.0f)
    {
        return;
    }

    bIsRunning = true;
    UpdateMovementSpeed();

    const float ConsumptionInterval = FMath::Max(RunStaminaConsumptionInterval, 0.01f);

    GetWorldTimerManager().SetTimer(
        RunStaminaTimerHandle,
        this,
        &APlayerCharacter::ConsumeRunStamina,
        ConsumptionInterval,
        true
    );
}

void APlayerCharacter::StopRun()
{
    GetWorldTimerManager().ClearTimer(RunStaminaTimerHandle);

    bIsRunning = false;
    UpdateMovementSpeed();
}

void APlayerCharacter::ConsumeRunStamina()
{
    if (!bIsRunning || !IsValid(StatComponent))
    {
        StopRun();
        return;
    }

    // Shift 눌러도 실제로 이동 중일 때만 소모하기
    const UCharacterMovementComponent* Movement = GetCharacterMovement();

    if (!IsValid(Movement) || Movement->Velocity.SizeSquared2D() <= UE_KINDA_SMALL_NUMBER)
    {
        return;
    }

    if (StatComponent->HasEnoughStamina(RunStaminaCostPerTick))
    {
        StatComponent->UseStamina(RunStaminaCostPerTick);
    }
    else
    {
        const float RemainingStamina = StatComponent->GetCurrentStamina();
        if (RemainingStamina > 0.0f)
        {
            StatComponent->UseStamina(RemainingStamina);
        }
    }

    if (StatComponent->GetCurrentStamina() <= UE_KINDA_SMALL_NUMBER)
    {
        StopRun();
    }
}

void APlayerCharacter::UpdateMovementSpeed()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();

    if (!Movement) { return; }
    Movement->MaxWalkSpeed = bIsRunning ? RunSpeed : WalkSpeed;
}

void APlayerCharacter::StartRoll()
{
    UE_LOG(LogTemp, Log, TEXT("APlayerCharacter::StartRoll - 호출됨"));

    if (!IsValid(CombatComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::StartRoll - CombatComponent가 유효하지 않습니다."));
        return;
    }

    CombatComponent->Roll();
}

