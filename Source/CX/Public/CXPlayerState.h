#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXGameTypes.h"
#include "CXPlayerState.generated.h"

/// 개인 코인과 속도 보너스를 서버에서 관리합니다.
UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
    GENERATED_BODY()
  public:
    /// 소유 클라이언트에게만 개인 속성을 복제하도록 등록합니다.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    /// 현재 원화를 반환합니다.
    UFUNCTION(BlueprintPure, Category = "과제")
    int32 GetCoins() const
    {
        return Coins;
    }
    /// 추가 최대 속도 값을 반환합니다.
    UFUNCTION(BlueprintPure, Category = "과제")
    float GetBonusMaxSpeed() const
    {
        return BonusMaxSpeed;
    }
    /// 서버에서 양수 원화를 안전하게 지급합니다.
    bool AddCoins(int32 Amount);
    /// 잔액이 충분한 경우 서버에서 원화를 차감합니다.
    bool TrySpendCoins(int32 Amount);
    /// 서버에서 속도 보너스를 증가시킵니다.
    bool AddBonusMaxSpeed(float Amount);
    /// 서버에서 수신자와 지급자를 함께 검증한 뒤 원화를 이전합니다.
    bool TransferCoinsTo(ACXPlayerState* Receiver, int32 Amount);
    /// 로컬 화면이 개인 상태 변경을 구독합니다.
    FCXStateChanged OnChanged;

  private:
    /// 소유 클라이언트에만 전달하는 보유 원화입니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    int32 Coins = 0;
    /// 소유 클라이언트에만 전달하는 추가 최대 속도입니다.
    UPROPERTY(ReplicatedUsing = NotifyChanged)
    float BonusMaxSpeed = 0.0f;
    /// 수신한 개인 상태의 변경을 화면에 알립니다.
    UFUNCTION()
    void NotifyChanged();
};
