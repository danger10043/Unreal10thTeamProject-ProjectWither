// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Component/StatComponent.h"
#include "Component/WeaponComponent.h"
#include "Component/CombatComponent.h"
#include "Component/InventoryComponent.h"
#include "Equipment/EquipmentComponent.h"
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
#include "Blueprint/UserWidget.h"

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
    EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
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

void APlayerCharacter::ToggleInventory()
{
    if (bIsInventoryOpen)
    {
        CloseInventory();
        return;
    }

    OpenInventory();
}

void APlayerCharacter::OpenInventory()
{
    if (bIsInventoryOpen || !IsValid(InventoryScreenClass))										// 이미 열려 있거나 생성할 인벤토리 UI 클래스가 없으면 처리하지 않는다.
    {
        return;
    }

    APlayerController* PlayerController = Cast<APlayerController>(GetController());					// 입력 모드와 마우스 커서를 제어할 PlayerController를 가져온다.

    if (!IsValid(PlayerController))
    {
        return;
    }

    InventoryScreenWidget = CreateWidget<UUserWidget>(PlayerController, InventoryScreenClass);		// 지정된 인벤토리 UI 클래스로 위젯을 생성한다.

    if (!IsValid(InventoryScreenWidget))
    {
        return;
    }

    InventoryScreenWidget->AddToViewport();														// 생성된 인벤토리 UI를 화면에 표시한다.
    bIsInventoryOpen = true;																		// 인벤토리가 열려 있는 상태로 갱신한다.

    PlayerController->bShowMouseCursor = true;														// 인벤토리 조작을 위해 마우스 커서를 표시한다.

    FInputModeUIOnly InputMode;																		// 인벤토리가 열려 있는 동안 UI 입력만 받을 수 있도록 설정한다.
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);							// 마우스가 뷰포트 안에 강제로 잠기지 않도록 한다.
    InputMode.SetWidgetToFocus(InventoryScreenWidget->TakeWidget());								// 인벤토리 UI가 키보드/마우스 포커스를 받도록 한다.
    PlayerController->SetInputMode(InputMode);														// 설정한 입력 모드를 PlayerController에 적용한다.
}

void APlayerCharacter::CloseInventory()
{
    if (!bIsInventoryOpen)																			// 인벤토리가 열려 있지 않으면 처리하지 않는다.
    {
        return;
    }

    if (IsValid(InventoryScreenWidget))
    {
        InventoryScreenWidget->RemoveFromParent();													// 화면에 표시된 인벤토리 UI를 제거한다.
        InventoryScreenWidget = nullptr;															// 제거된 위젯 참조를 비운다.
    }

    bIsInventoryOpen = false;																		// 인벤토리가 닫힌 상태로 갱신한다.

    APlayerController* PlayerController = Cast<APlayerController>(GetController());					// 게임 입력 모드를 복구할 PlayerController를 가져온다.

    if (!IsValid(PlayerController))
    {
        return;
    }

    PlayerController->bShowMouseCursor = false;														// 게임 조작 상태로 돌아가면서 마우스 커서를 숨긴다.

    FInputModeGameOnly InputMode;																	// 게임 입력만 받는 입력 모드로 복구한다.
    PlayerController->SetInputMode(InputMode);														// 설정한 입력 모드를 PlayerController에 적용한다.
}

UStatComponent* APlayerCharacter::GetStatComponent_Implementation() const
{
    return StatComponent;
}

UWeaponComponent* APlayerCharacter::GetWeaponComponent_Implementation() const
{
    return WeaponComponent;
}

UEquipmentComponent* APlayerCharacter::GetEquipmentComponent_Implementation() const
{
    return EquipmentComponent;
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
    EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleInventory);
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

    if (!IsValid(CombatComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("APlayerCharacter::StartRoll - CombatComponent가 유효하지 않습니다."));
        return;
    }
    CombatComponent->Roll();
}

void APlayerCharacter::AttackInput()
{
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
            const int32 AddedQuantity = InventoryComponent->AddItem(TestGunData, 1);

            if (AddedQuantity > 0)
            {
                const int32 GunSlot = InventoryComponent->FindItemSlot(TestGunData->GetItemId());

                FItemInstance TestGunInstance;

                if (GunSlot != INDEX_NONE && InventoryComponent->GetItemAtSlot(GunSlot, TestGunInstance))
                {
                    TestGunInstance.CurrentAmmo = FMath::Max(0, TestGunData->GetMaxAmmo());
                    InventoryComponent->UpdataItemAtSlot(GunSlot, TestGunInstance);
                }
            }
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

