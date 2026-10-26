#include "CXCourseHUD.h"
#include "CXPlayerState.h"
#include "CXGameState.h"
#include "ChatUI.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

/// 복제 객체가 준비될 때까지 상태 변경 알림의 연결을 재시도합니다.
void UCXCourseHUD::NativeConstruct()
{
    Super::NativeConstruct();
    BindState();
    if (IsValid(GetWorld()) && (!BoundPlayer.IsValid() || !BoundGame.IsValid()))
    {
        GetWorld()->GetTimerManager().SetTimer(BindTimer, this, &UCXCourseHUD::BindState, 0.2f, true);
    }
}

/// 화면 종료 시 모든 상태 구독과 준비 타이머를 해제합니다.
void UCXCourseHUD::NativeDestruct()
{
    if (BoundPlayer.IsValid())
    {
        BoundPlayer->OnChanged.RemoveAll(this);
    }
    if (BoundGame.IsValid())
    {
        BoundGame->OnChanged.RemoveAll(this);
    }
    if (IsValid(GetWorld()))
    {
        GetWorld()->GetTimerManager().ClearTimer(BindTimer);
    }
    BoundPlayer.Reset();
    BoundGame.Reset();
    Super::NativeDestruct();
}

/// 복제 객체가 준비되는 즉시 구독하며 준비 후에는 반복 조회를 중단합니다.
void UCXCourseHUD::BindState()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!IsValid(PlayerController) || !IsValid(GetWorld()))
    {
        return;
    }
    ACXPlayerState* PlayerState = PlayerController->GetPlayerState<ACXPlayerState>();
    ACXGameState* GameState = GetWorld()->GetGameState<ACXGameState>();
    if (IsValid(PlayerState) && BoundPlayer.Get() != PlayerState)
    {
        if (BoundPlayer.IsValid())
        {
            BoundPlayer->OnChanged.RemoveAll(this);
        }
        BoundPlayer = PlayerState;
        PlayerState->OnChanged.AddUObject(this, &UCXCourseHUD::Refresh);
    }
    if (IsValid(GameState) && BoundGame.Get() != GameState)
    {
        if (BoundGame.IsValid())
        {
            BoundGame->OnChanged.RemoveAll(this);
        }
        BoundGame = GameState;
        GameState->OnChanged.AddUObject(this, &UCXCourseHUD::Refresh);
    }
    if (BoundPlayer.IsValid() && BoundGame.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(BindTimer);
    }
    Refresh();
}

/// 세 종류의 표시를 각각 담당 함수에 맡깁니다.
void UCXCourseHUD::Refresh()
{
    RefreshPersonalInfo();
    RefreshRoundInfo();
    RefreshResult();
}

/// 소유 플레이어에게 복제된 개인 수치만 표시합니다.
void UCXCourseHUD::RefreshPersonalInfo()
{
    if (!BoundPlayer.IsValid() || !IsValid(PersonalText))
    {
        return;
    }
    const FText Info = FText::FromString(FString::Printf(
        TEXT("코인 %d개 | 추가 최대 속도 %.0f"), BoundPlayer->GetCoins(), BoundPlayer->GetBonusMaxSpeed()));
    PersonalText->SetText(Info);
}

/// 서버에서 복제한 라운드 상태와 남은 시간을 표시합니다.
void UCXCourseHUD::RefreshRoundInfo()
{
    if (!BoundGame.IsValid() || !IsValid(RoundText))
    {
        return;
    }
    FString PhaseLabel;
    switch (BoundGame->GetPhase())
    {
    case ECXRoundPhase::Waiting:
        PhaseLabel = TEXT("대기");
        break;
    case ECXRoundPhase::Playing:
        PhaseLabel = TEXT("진행");
        break;
    case ECXRoundPhase::Finished:
        PhaseLabel = TEXT("종료");
        break;
    }
    RoundText->SetText(FText::FromString(
        FString::Printf(TEXT("%s | 남은 시간 %d초"), *PhaseLabel, BoundGame->GetRemainingSeconds())));
}

/// 종료 단계에서 내 식별자가 우승자 목록에 있는지 확인합니다.
void UCXCourseHUD::RefreshResult()
{
    if (!BoundGame.IsValid() || !IsValid(ResultText))
    {
        return;
    }
    if (BoundGame->GetPhase() != ECXRoundPhase::Finished)
    {
        ResultText->SetText(FText::GetEmpty());
        return;
    }
    if (!BoundPlayer.IsValid())
    {
        ResultText->SetText(FText::FromString(TEXT("관전 종료")));
        return;
    }
    const bool bIsWinner = BoundGame->GetWinnerIds().Contains(BoundPlayer->GetPlayerId());
    FString ResultLabel = TEXT("패배");
    if (bIsWinner)
    {
        ResultLabel = TEXT("승리");
        if (BoundGame->GetWinnerIds().Num() > 1)
        {
            ResultLabel = TEXT("공동 우승");
        }
    }
    const FString WinnerNames = FString::Join(BoundGame->GetWinnerNames(), TEXT(", "));
    ResultText->SetText(
        FText::FromString(FString::Printf(TEXT("%s | 우승자: %s"), *ResultLabel, *WinnerNames)));
}
