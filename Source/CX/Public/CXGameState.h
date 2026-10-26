#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameTypes.h"
#include "CXGameState.generated.h"

class UParticleSystem;
class USoundBase;

/// 모든 참가자에게 라운드 상태와 남은 시간 및 우승자를 전달합니다.
UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
    GENERATED_BODY()
  public:
    /// 공용 연출의 기본 파티클과 사운드를 준비합니다.
    ACXGameState();
    /// 공용 라운드 상태를 모든 연결에 복제합니다.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    /// 현재 라운드 단계를 반환합니다.
    ECXRoundPhase GetPhase() const
    {
        return Phase;
    }
    /// 표시할 남은 시간을 반환합니다.
    int32 GetRemainingSeconds() const
    {
        return RemainingSeconds;
    }
    /// 서버가 확정한 우승자 식별자 목록을 반환합니다.
    const TArray<int32>& GetWinnerIds() const
    {
        return WinnerIds;
    }
    /// 서버가 확정한 우승자 표시 이름을 반환합니다.
    const TArray<FString>& GetWinnerNames() const
    {
        return WinnerNames;
    }
    /// 서버에서 공용 단계와 시간을 갱신합니다.
    void SetRound(ECXRoundPhase NewPhase, int32 Seconds);
    /// 서버에서 공동 우승자 전체를 확정합니다.
    void SetWinners(const TArray<int32>& WinnerPlayerIds, const TArray<FString>& WinnerPlayerNames);
    /// 모든 클라이언트와 리슨 호스트에서 획득 및 피격 연출을 재생합니다.
    UFUNCTION(NetMulticast, Reliable)
    void MulticastEffect(FVector Location, bool bPickup);
    /// 서버가 사용한 추적선을 각 화면에 표시합니다.
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastAttackLine(FVector Start, FVector End, bool bHit);
    /// 공용 상태 변경을 화면에 알립니다.
    FCXStateChanged OnChanged;

  private:
    /// 모든 참가자가 공유하는 현재 단계입니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    ECXRoundPhase Phase = ECXRoundPhase::Waiting;
    /// 서버가 매초 감소시키는 남은 시간입니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    int32 RemainingSeconds = 0;
    /// 이름 중복과 관계없이 우승자를 구분하는 식별자 목록입니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    TArray<int32> WinnerIds;
    /// 종료 시점의 우승자 이름을 보관합니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    TArray<FString> WinnerNames;
    /// 획득과 피격 위치에서 재생하는 기본 파티클입니다.
    UPROPERTY(EditDefaultsOnly, Category = "연출")
    TObjectPtr<UParticleSystem> Effect;
    /// 아이템 획득 위치에서 재생하는 기본 사운드입니다.
    UPROPERTY(EditDefaultsOnly, Category = "연출")
    TObjectPtr<USoundBase> PickupSound;
    /// 복제된 공용 상태의 변경을 화면에 알립니다.
    UFUNCTION()
    void NotifyChanged();
};
