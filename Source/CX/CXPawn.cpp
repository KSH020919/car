/// 원본 저작권은 Epic Games, Inc.에 있으며 모든 권리를 보유합니다.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "CXPlayerState.h"
#include "CXGameState.h"
#include "CXGameTypes.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

/// 차량과 카메라 및 범퍼 기준점을 구성합니다.
ACXPawn::ACXPawn()
{
    bReplicates = true;
    FrontBumper = CreateDefaultSubobject<USceneComponent>(TEXT("FrontBumper"));
    FrontBumper->SetupAttachment(GetRootComponent());
    FrontBumper->SetRelativeLocation(FVector(250.0f, 0.0f, 45.0f));
    AttackAction = CreateDefaultSubobject<UInputAction>(TEXT("NativeAttackAction"));
    AttackAction->ValueType = EInputActionValueType::Boolean;
    if (FPackageName::DoesPackageExist(TEXT("/Game/Course/Input/IA_Attack")))
    {
        static ConstructorHelpers::FObjectFinder<UInputAction> Asset(
            TEXT("/Game/Course/Input/IA_Attack.IA_Attack"));
        if (Asset.Succeeded())
        {
            AttackAction = Asset.Object;
        }
    }
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
    FrontSpringArm->SetupAttachment(GetMesh());
    FrontSpringArm->TargetArmLength = 0.0f;
    FrontSpringArm->bDoCollisionTest = false;
    FrontSpringArm->bEnableCameraRotationLag = true;
    FrontSpringArm->CameraRotationLagSpeed = 15.0f;
    FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

    FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
    FrontCamera->SetupAttachment(FrontSpringArm);
    FrontCamera->bAutoActivate = false;

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
    BackSpringArm->SetupAttachment(GetMesh());
    BackSpringArm->TargetArmLength = 650.0f;
    BackSpringArm->SocketOffset.Z = 150.0f;
    BackSpringArm->bDoCollisionTest = false;
    BackSpringArm->bInheritPitch = false;
    BackSpringArm->bInheritRoll = false;
    BackSpringArm->bEnableCameraRotationLag = true;
    BackSpringArm->CameraRotationLagSpeed = 2.0f;
    BackSpringArm->CameraLagMaxDistance = 50.0f;

    BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
    BackCamera->SetupAttachment(BackSpringArm);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(FName("Vehicle"));
    GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    GetMesh()->SetGenerateOverlapEvents(true);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

/// 차량 입력과 공격 입력을 연결합니다.
void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        /// 공격 액션을 한 번 누를 때 한 번 요청하도록 연결합니다.
        if (IsValid(AttackAction))
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this,
                                               &ACXPawn::InputAttack);
            APlayerController* PlayerController = Cast<APlayerController>(GetController());
            ULocalPlayer* Local = IsValid(PlayerController) ? PlayerController->GetLocalPlayer() : nullptr;
            UEnhancedInputLocalPlayerSubsystem* Subsystem =
                IsValid(Local) ? Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
            if (IsValid(Subsystem))
            {
                AttackMapping = NewObject<UInputMappingContext>(this);
                AttackMapping->MapKey(AttackAction, EKeys::LeftMouseButton);
                Subsystem->AddMappingContext(AttackMapping, 1);
            }
        }
        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this,
                                           &ACXPawn::Steering);
        EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this,
                                           &ACXPawn::Steering);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this,
                                           &ACXPawn::Throttle);
        EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this,
                                           &ACXPawn::Throttle);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this,
                                           &ACXPawn::StartHandbrake);
        EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this,
                                           &ACXPawn::StopHandbrake);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this,
                                           &ACXPawn::LookAround);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this,
                                           &ACXPawn::ToggleCamera);

        /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
        EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this,
                                           &ACXPawn::ResetVehicle);
    }
    else
    {
        UE_LOG(LogTemplateVehicle, Error,
               TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the "
                    "Enhanced Input system. If you intend to use the legacy system, then you will need to "
                    "update this C++ file."),
               *GetNameSafe(this));
    }
}

/// 기존 카메라 복귀와 공중 감쇠만 갱신합니다.
void ACXPawn::Tick(float Delta)
{
    Super::Tick(Delta);
    if (!IsValid(ChaosVehicleMovement) || !IsValid(GetMesh()) || !IsValid(BackSpringArm))
    {
        return;
    }

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
    GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
    CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

    BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

/// 차량 조향 입력을 전달합니다.
void ACXPawn::Steering(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    float SteeringValue = Value.Get<float>();

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

/// 차량 가속 입력을 전달합니다.
void ACXPawn::Throttle(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    float ThrottleValue = Value.Get<float>();

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

/// 차량 제동 입력을 전달합니다.
void ACXPawn::Brake(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    float BreakValue = Value.Get<float>();

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

/// 제동등을 켭니다.
void ACXPawn::StartBrake(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BrakeLights(true);
}

/// 제동등과 제동 입력을 해제합니다.
void ACXPawn::StopBrake(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BrakeLights(false);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetBrakeInput(0.0f);
}

/// 주차 브레이크와 제동등을 켭니다.
void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetHandbrakeInput(true);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BrakeLights(true);
}

/// 주차 브레이크와 제동등을 해제합니다.
void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    ChaosVehicleMovement->SetHandbrakeInput(false);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BrakeLights(false);
}

/// 카메라 둘러보기 입력을 반영합니다.
void ACXPawn::LookAround(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    float LookValue = Value.Get<float>();

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

/// 전방과 후방 카메라를 전환합니다.
void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    bFrontCameraActive = !bFrontCameraActive;

    FrontCamera->SetActive(bFrontCameraActive);
    BackCamera->SetActive(!bFrontCameraActive);
}

/// 차량을 정방향으로 복구합니다.
void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    FRotator ResetRotation = GetActorRotation();
    ResetRotation.Pitch = 0.0f;
    ResetRotation.Roll = 0.0f;

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.
    SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr,
                      ETeleportType::TeleportPhysics);

    GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

    UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

#undef LOCTEXT_NAMESPACE

/// 소유 차량의 로컬 입력을 서버에 요청합니다.
void ACXPawn::InputAttack()
{
    if (IsLocallyControlled())
    {
        CTSAttack();
    }
}

/// 서버 시간과 진행 상태를 검사한 뒤 범퍼 중앙에서 전방을 추적합니다.
void ACXPawn::CTSAttack_Implementation()
{
    if (!HasAuthority() || !IsValid(GetWorld()) || !IsValid(FrontBumper))
    {
        return;
    }
    ACXGameState* GameState = GetWorld()->GetGameState<ACXGameState>();
    if (!IsValid(GameState) || GameState->GetPhase() != ECXRoundPhase::Playing)
    {
        return;
    }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastAttackTime < FMath::Max(0.1f, AttackCooldown))
    {
        return;
    }
    LastAttackTime = Now;
    const FVector Start = FrontBumper->GetComponentLocation();
    const FVector End = Start + GetActorForwardVector() * AttackRange;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
    GameState->MulticastAttackLine(Start, bHit ? Hit.ImpactPoint : End, bHit);
    ACXPawn* Victim = bHit ? Cast<ACXPawn>(Hit.GetActor()) : nullptr;
    if (!IsValid(Victim) || Victim == this)
    {
        return;
    }
    const float Applied = UGameplayStatics::ApplyDamage(Victim, static_cast<float>(CXRules::StealAmount),
                                                        GetController(), this, UDamageType::StaticClass());
    if (Applied > 0.0f)
    {
        GameState->MulticastEffect(Hit.ImpactPoint, false);
    }
}

/// 잔액 부족과 자기 공격을 차단하고 코인 이전 성공 시 피해 이벤트를 발생시킵니다.
float ACXPawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
                          AActor* DamageCauser)
{
    if (!HasAuthority() || !IsValid(GetWorld()) || !CanBeDamaged() || !FMath::IsFinite(DamageAmount) ||
        DamageAmount <= 0.0f)
    {
        return 0.0f;
    }
    ACXGameState* GameState = GetWorld()->GetGameState<ACXGameState>();
    ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);
    if (!IsValid(GameState) || GameState->GetPhase() != ECXRoundPhase::Playing || !IsValid(Attacker) ||
        Attacker == this || EventInstigator != Attacker->GetController())
    {
        return 0.0f;
    }
    ACXPlayerState* VictimPlayerState = GetPlayerState<ACXPlayerState>();
    ACXPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ACXPlayerState>();
    if (!IsValid(VictimPlayerState) || !IsValid(AttackerPlayerState) ||
        !VictimPlayerState->TransferCoinsTo(AttackerPlayerState, CXRules::StealAmount))
    {
        return 0.0f;
    }
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
