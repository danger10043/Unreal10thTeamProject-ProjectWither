// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Component/CombatComponent.h"
#include "Component/InventoryComponent.h"
#include "DataAsset/WeaponDataAsset.h"
#include "Widget/TestMainUIWidget.h"

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
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
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
	
    AddDefaultTestWeapons();

    if (IsLocallyControlled() && IsValid(TestMainUIClass))
    {
        APlayerController* PlayerController =
            Cast<APlayerController>(GetController());

        if (IsValid(PlayerController))
        {
            TestMainUIInstance = CreateWidget<UTestMainUIWidget>(
                PlayerController,
                TestMainUIClass
            );

            if (IsValid(TestMainUIInstance))
            {
                TestMainUIInstance->AddToViewport();
            }
        }
    }

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

    if (IsValid(TestMainUIInstance))
    {
        TestMainUIInstance->RemoveFromParent();
        TestMainUIInstance = nullptr;
    }

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
    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

    EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
    
    EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::StartRun);
    EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRun);
    EnhancedInput->BindAction(RunAction, ETriggerEvent::Canceled, this, &APlayerCharacter::StopRun);

    EnhancedInput->BindAction(RollAction, ETriggerEvent::Started, this, &APlayerCharacter::StartRoll);

	EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::AttackInput);
    EnhancedInput->BindAction(BlockAction, ETriggerEvent::Started, this, &APlayerCharacter::StartBlockInput);
    EnhancedInput->BindAction(BlockAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopBlockInput);
    EnhancedInput->BindAction(BlockAction, ETriggerEvent::Canceled, this, &APlayerCharacter::StopBlockInput);
    EnhancedInput->BindAction(SwapWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::SwapWeaponInput);
}

float APlayerCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float IncomingDamage = Super::TakeDamage(
        Damage,
        DamageEvent,
        EventInstigator,
        DamageCauser
    );

    if (!IsValid(CombatComponent)) { return 0.0f; }

    return CombatComponent->ReceiveHit(IncomingDamage, DamageCauser, EventInstigator);
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

void APlayerCharacter::AttackInput()
{
	UE_LOG(LogTemp, Log, TEXT("APlayerCharacter::AttackInput - 플레이어 공격 입력"));
    if (!IsValid(CombatComponent))
    {
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::AttackInput - CombatComponent가 유효하지 않습니다."));
        return; 
    }
    CombatComponent->Attack();
}

void APlayerCharacter::StartBlockInput()
{
    if (!IsValid(CombatComponent)) { return; }
    CombatComponent->StartBlock();
}

void APlayerCharacter::StopBlockInput()
{
    if (!IsValid(CombatComponent)) { return; }
    CombatComponent->StopBlock();
}

void APlayerCharacter::SwapWeaponInput()
{
    if (!IsValid(WeaponComponent)) return;

    if (!WeaponComponent->SwapWeapon())
    {
		UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::SwapWeaponInput - 무기 교체에 실패했습니다."));
    }
}

void APlayerCharacter::AddDefaultTestWeapons()
{
    if (!IsValid(InventoryComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::AddDefaultTestWeapons - InventoryComponent가 유효하지 않습니다."));
        return;
    }

    if (IsValid(TestSwordData))
    {
        if (TestSwordData->GetWeaponType() != EWeaponType::Sword)
        {
            UE_LOG(LogTemp, Warning, TEXT("DefaultTestSword에 검이 아닌 WeaponDataAsset이 지정되어 있습니다."));
        }
        else if (!InventoryComponent->HasItem(TestSwordData->GetItemId()))
        {
            InventoryComponent->AddItem(TestSwordData, 1);
        }
    }

    if (IsValid(TestGunData))
    {
        if (TestGunData->GetWeaponType() != EWeaponType::Gun)
        {
            UE_LOG(LogTemp, Warning, TEXT("DefaultTestGun에 총이 아닌 WeaponDataAsset이 지정되어 있습니다."));
        }
        else if (!InventoryComponent->HasItem(TestGunData->GetItemId()))
        {
            InventoryComponent->AddItem(TestGunData, 1);
        }
	}

    if (bEquipTestWeapon && IsValid(TestSwordData) && IsValid(WeaponComponent))
    {
        if (!WeaponComponent->EquipWeapon(TestSwordData))
        {
            UE_LOG(LogTemp, Warning, TEXT("기본 테스트 검 자동 장착에 실패했습니다."));
        }
    }
}

