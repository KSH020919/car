#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "CXCourseHUD.h"
#include "CXPlayerState.h"
#include "ChatUI.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"

/// 전용 서버에는 화면을 생성하지 않습니다.
void ACXPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController())
    {
        return;
    }
    if (VehicleUIClass)
    {
        VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);
    }
    if (IsValid(VehicleUI))
    {
        VehicleUI->AddToViewport(0);
    }
    /// 지정한 블루프린트만 생성하며 코드로 화면 배치를 만들지 않습니다.
    if (CourseHUDClass)
    {
        CourseHUD = CreateWidget<UCXCourseHUD>(this, CourseHUDClass);
    }
    if (IsValid(CourseHUD))
    {
        CourseHUD->AddToViewport(10);
        if (IsValid(CourseHUD->Chat))
        {
            for (const FCXPendingChat& Entry : PendingChat)
            {
                CourseHUD->Chat->OnNewChatMessage(Entry.Sender, Entry.Message);
            }
            PendingChat.Empty();
        }
    }

    if (IsValid(VehicleUI))
    {
        GetWorldTimerManager().SetTimer(VehicleTimer, this, &ACXPlayerController::RefreshVehicleUI, 0.1f,
                                        true);
    }
}
/// 기존 차량 컨텍스트와 채팅 단축키를 등록합니다.
void ACXPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    ULocalPlayer* Local = GetLocalPlayer();
    if (IsValid(Local))
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem =
            Local->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
        if (IsValid(Subsystem))
        {
            if (IsValid(InputMappingContext))
            {
                Subsystem->AddMappingContext(InputMappingContext, 0);
            }
            if (bUseSteeringWheelControls && IsValid(SteeringWheelInputMappingContext))
            {
                Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
            }
        }
    }
    if (IsValid(InputComponent))
    {
        InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ACXPlayerController::FocusChat);
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ACXPlayerController::ToggleMouse);
    }
}
/// 화면 갱신 타이머를 해제합니다.
void ACXPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(VehicleTimer);
    Super::EndPlay(EndPlayReason);
}
/// 늦게 준비된 개인 상태를 연결합니다.
void ACXPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    if (IsValid(CourseHUD))
    {
        CourseHUD->BindState();
    }
}
/// 내용과 길이 및 간격을 서버에서 검증합니다.
void ACXPlayerController::CTSChatMessage_Implementation(const FString& Message)
{
    if (!IsValid(GetWorld()) || !IsValid(PlayerState))
    {
        return;
    }
    FString Clean = Message.TrimStartAndEnd();
    Clean.ReplaceInline(TEXT("\r"), TEXT(" "));
    Clean.ReplaceInline(TEXT("\n"), TEXT(" "));
    const double Now = GetWorld()->GetTimeSeconds();
    if (Clean.IsEmpty() || Clean.Len() > 200 || Now - LastChatTime < 0.25)
    {
        return;
    }
    LastChatTime = Now;
    const FString Sender = PlayerState->GetPlayerName();
    for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
    {
        if (IsValid(*It))
        {
            It->STCChatMessage(Sender, Clean);
        }
    }
}
/// 화면 준비 전 메시지를 대기열로 보호합니다.
void ACXPlayerController::STCChatMessage_Implementation(const FString& Sender, const FString& Message)
{

    if (IsValid(CourseHUD) && IsValid(CourseHUD->Chat))
    {
        CourseHUD->Chat->OnNewChatMessage(Sender, Message);
        return;
    }
    if (PendingChat.Num() >= 100)
    {
        PendingChat.RemoveAt(0);
    }
    PendingChat.Add({Sender, Message});
}
/// 로컬 채팅 입력을 활성화합니다.
void ACXPlayerController::FocusChat()
{
    if (IsValid(CourseHUD) && IsValid(CourseHUD->Chat))
    {
        CourseHUD->Chat->FocusInput();
    }
}
/// 위젯 클릭과 차량 조작 사이의 입력을 전환합니다.
void ACXPlayerController::ToggleMouse()
{
    bShowMouseCursor = !bShowMouseCursor;
    if (bShowMouseCursor)
    {
        SetInputMode(FInputModeGameAndUI());
    }
    else
    {
        SetInputMode(FInputModeGameOnly());
    }
}
/// 기존 속도계에 간단한 수치만 전달합니다.
void ACXPlayerController::RefreshVehicleUI()
{
    ACXPawn* VehiclePawn = GetPawn<ACXPawn>();
    if (!IsValid(VehiclePawn) || !IsValid(VehicleUI))
    {
        return;
    }
    UChaosWheeledVehicleMovementComponent* Movement = VehiclePawn->GetChaosVehicleMovement();
    if (!IsValid(Movement))
    {
        return;
    }
    VehicleUI->UpdateSpeed(Movement->GetForwardSpeed());
    VehicleUI->UpdateGear(Movement->GetCurrentGear());
}
