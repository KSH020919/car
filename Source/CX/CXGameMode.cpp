#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "CXPlayerState.h"
#include "CXGameState.h"
#include "CXGameTypes.h"
#include "CXPawn.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

/// 기존 차량 조작 애셋과 새로운 상태 클래스를 지정합니다.
ACXGameMode::ACXGameMode()
{
    PlayerControllerClass = ACXPlayerController::StaticClass();
    PlayerStateClass = ACXPlayerState::StaticClass();
    GameStateClass = ACXGameState::StaticClass();
    static ConstructorHelpers::FClassFinder<APawn> Vehicle(
        TEXT("/Game/VehicleTemplate/Blueprints/SportsCar/BP_SportsCar_Pawn"));
    if (Vehicle.Succeeded())
    {
        DefaultPawnClass = Vehicle.Class;
    }
    static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(
        TEXT("/Game/VehicleTemplate/Blueprints/BP_VehicleAdvPlayerController"));
    if (PlayerController.Succeeded())
    {
        PlayerControllerClass = PlayerController.Class;
    }
}
/// 첫 플레이어가 오기 전에는 카운트다운을 실행하지 않습니다.
void ACXGameMode::BeginPlay()
{
    Super::BeginPlay();
    ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    if (IsValid(RoundGameState) && !bStarted)
    {
        RoundGameState->SetRound(ECXRoundPhase::Waiting, 0);
    }
}
/// 처음 들어온 플레이어만 대기 타이머를 시작합니다.
void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    /// 서버에서 새 참가자에게 초기 코인 50개를 한 번 지급합니다.
    if (!HasAuthority() || !IsValid(NewPlayer))
    {
        return;
    }
    ACXPlayerState* JoiningPlayerState = NewPlayer->GetPlayerState<ACXPlayerState>();
    if (IsValid(JoiningPlayerState))
    {
        JoiningPlayerState->AddCoins(CXRules::StartingCoins);
    }

    ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    if (!IsValid(RoundGameState) || !IsValid(NewPlayer) || bStarted ||
        RoundGameState->GetPhase() != ECXRoundPhase::Waiting)
    {
        return;
    }
    bStarted = true;
    RoundGameState->SetRound(ECXRoundPhase::Waiting, FMath::Max(1, WaitingSeconds));
    GetWorldTimerManager().SetTimer(RoundTimer, this, &ACXGameMode::AdvanceSecond, 1.0f, true);
}
/// 다른 참가자가 남아 있으면 진행 시간을 유지합니다.
void ACXGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    if (GetNumPlayers() > 0)
    {
        return;
    }
    GetWorldTimerManager().ClearTimer(RoundTimer);
    bStarted = false;
    ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    if (IsValid(RoundGameState))
    {
        RoundGameState->SetWinners({}, {});
        RoundGameState->SetRound(ECXRoundPhase::Waiting, 0);
    }
}
/// 전용 서버의 공용 단계를 기준으로 판정합니다.
bool ACXGameMode::IsPlaying() const
{
    const ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    return IsValid(RoundGameState) && RoundGameState->GetPhase() == ECXRoundPhase::Playing;
}
/// 대기 종료와 플레이 종료를 정확히 한 번 전환합니다.
void ACXGameMode::AdvanceSecond()
{
    ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    if (!IsValid(RoundGameState))
    {
        return;
    }
    const int32 Remaining = FMath::Max(0, RoundGameState->GetRemainingSeconds() - 1);
    if (Remaining > 0)
    {
        RoundGameState->SetRound(RoundGameState->GetPhase(), Remaining);
        return;
    }
    if (RoundGameState->GetPhase() == ECXRoundPhase::Waiting)
    {
        RoundGameState->SetRound(ECXRoundPhase::Playing, FMath::Max(1, PlayingSeconds));
    }
    else if (RoundGameState->GetPhase() == ECXRoundPhase::Playing)
    {
        FinishRound();
    }
}
/// 서버의 모든 현재 참가자를 비교하고 동점자를 전부 포함합니다.
void ACXGameMode::FinishRound()
{
    GetWorldTimerManager().ClearTimer(RoundTimer);
    ACXGameState* RoundGameState = GetGameState<ACXGameState>();
    if (!IsValid(RoundGameState))
    {
        return;
    }
    int32 HighestCoins = -1;
    TArray<int32> WinnerPlayerIds;
    TArray<FString> WinnerPlayerNames;
    for (APlayerState* BaseState : RoundGameState->PlayerArray)
    {
        ACXPlayerState* PlayerState = Cast<ACXPlayerState>(BaseState);
        if (!IsValid(PlayerState) || PlayerState->IsOnlyASpectator())
        {
            continue;
        }
        if (PlayerState->GetCoins() > HighestCoins)
        {
            HighestCoins = PlayerState->GetCoins();
            WinnerPlayerIds.Empty();
            WinnerPlayerNames.Empty();
        }
        if (PlayerState->GetCoins() == HighestCoins)
        {
            WinnerPlayerIds.Add(PlayerState->GetPlayerId());
            WinnerPlayerNames.Add(PlayerState->GetPlayerName());
        }
    }
    RoundGameState->SetWinners(WinnerPlayerIds, WinnerPlayerNames);
    RoundGameState->SetRound(ECXRoundPhase::Finished, 0);
}
/// 비어 있는 시작점을 고르고 모두 사용 중이면 엔진의 기본 선택을 사용합니다.
AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    TArray<APlayerStart*> Available;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        if (IsPlayerStartAvailable(*It))
        {
            Available.Add(*It);
        }
    }
    if (Available.IsEmpty())
    {
        return Super::ChoosePlayerStart_Implementation(Player);
    }
    return Available[FMath::RandHelper(Available.Num())];
}
/// 차량 크기에 맞는 박스로 기존 차량과의 겹침을 검사합니다.
bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* Start) const
{
    if (!IsValid(Start) || !IsValid(GetWorld()))
    {
        return false;
    }
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_Vehicle);
    return !GetWorld()->OverlapAnyTestByObjectType(Start->GetActorLocation() + FVector(0, 0, 80),
                                                   Start->GetActorQuat(), Objects,
                                                   FCollisionShape::MakeBox(FVector(250, 120, 80)));
}
/// 레벨을 떠날 때 라운드 타이머를 정리합니다.
void ACXGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(RoundTimer);
    Super::EndPlay(EndPlayReason);
}
