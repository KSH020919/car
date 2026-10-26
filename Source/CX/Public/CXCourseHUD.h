#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CXCourseHUD.generated.h"

class ACXPlayerState;
class ACXGameState;
class UTextBlock;
class UChatUI;

/// 개인 정보와 공용 시간 및 채팅과 승패를 이벤트로 표시합니다.
UCLASS()
class CX_API UCXCourseHUD : public UUserWidget
{
    GENERATED_BODY()
  public:
    /// 채팅 표시를 담당하는 자식 위젯입니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UChatUI> Chat;
    /// 준비된 상태 객체에 변경 알림을 연결합니다.
    void BindState();
    /// 현재 상태를 읽어 개인 정보와 결과 표시를 갱신합니다.
    void Refresh();

  protected:
    /// 준비 지연에 대응하는 짧은 구독 재시도를 시작합니다.
    virtual void NativeConstruct() override;
    /// 위젯 제거 시 구독과 재시도 타이머를 정리합니다.
    virtual void NativeDestruct() override;

  private:
    /// 내 코인과 추가 최대 속도를 표시합니다.
    void RefreshPersonalInfo();
    /// 현재 단계와 남은 시간을 표시합니다.
    void RefreshRoundInfo();
    /// 내 승패와 공동 우승자를 표시합니다.
    void RefreshResult();
    /// 원화와 보너스 수치를 표시합니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PersonalText;
    /// 단계와 남은 시간을 표시합니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> RoundText;
    /// 승패와 공동 우승자 목록을 표시합니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ResultText;
    /// 구독한 개인 상태가 살아 있는 동안만 참조합니다.
    TWeakObjectPtr<ACXPlayerState> BoundPlayer;
    /// 구독한 공용 상태가 살아 있는 동안만 참조합니다.
    TWeakObjectPtr<ACXGameState> BoundGame;
    /// 객체 준비 전의 제한적인 재시도에 사용합니다.
    FTimerHandle BindTimer;
};
