#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"
class APlayerStart;

/// 서버의 카운트다운과 승자 계산 및 시작 위치를 관리합니다.
UCLASS()
class CX_API ACXGameMode : public AGameModeBase
{
    GENERATED_BODY()
  public:
    /// 과제용 플레이어 상태와 공용 상태 및 차량을 지정합니다.
    ACXGameMode();
    /// 첫 접속에서만 대기 카운트다운을 시작합니다.
    virtual void PostLogin(APlayerController* NewPlayer) override;
    /// 참가자가 모두 떠나면 대기 상태로 초기화합니다.
    virtual void Logout(AController* Exiting) override;
    /// 진행 단계에서만 공격과 아이템 획득을 허용합니다.
    bool IsPlaying() const;

  protected:
    /// 공용 상태를 초기화합니다.
    virtual void BeginPlay() override;
    /// 겹치지 않는 차량 시작 지점을 선택합니다.
    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
    /// 게임 종료 시 타이머를 정리합니다.
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    /// 첫 접속 이후 대기할 초입니다.
    UPROPERTY(EditDefaultsOnly, Category = "라운드", meta = (ClampMin = "1"))
    int32 WaitingSeconds = 20;
    /// 실제 플레이를 진행할 초입니다.
    UPROPERTY(EditDefaultsOnly, Category = "라운드", meta = (ClampMin = "1"))
    int32 PlayingSeconds = 120;

  private:
    /// 초 단위 서버 타이머입니다.
    FTimerHandle RoundTimer;
    /// 타이머의 중복 시작을 방지합니다.
    bool bStarted = false;
    /// 초 단위로 시간을 감소시키고 단계 전환을 처리합니다.
    void AdvanceSecond();
    /// 현재 참가자의 최대 잔액과 공동 우승자를 계산합니다.
    void FinishRound();
    /// 시작 위치에 다른 차량이 있는지 검사합니다.
    bool IsPlayerStartAvailable(const APlayerStart* Start) const;
};
