#include "CXPlayerState.h"
#include "Net/UnrealNetwork.h"

/// 개인 상태를 소유 연결에만 복제합니다.
void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(ACXPlayerState, Coins, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ACXPlayerState, BonusMaxSpeed, COND_OwnerOnly);
}

/// 서버 지급의 권한과 양수 및 정수 범위를 검사합니다.
bool ACXPlayerState::AddCoins(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0 || Coins > MAX_int32 - Amount)
    {
        return false;
    }
    Coins += Amount;
    NotifyChanged();
    ForceNetUpdate();
    return true;
}

/// 잔액이 부족하면 값을 변경하지 않고 실패합니다.
bool ACXPlayerState::TrySpendCoins(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0 || Coins < Amount)
    {
        return false;
    }
    Coins -= Amount;
    NotifyChanged();
    ForceNetUpdate();
    return true;
}

/// 유한한 양수 속도 보너스만 서버에서 더합니다.
bool ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
    if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.0f ||
        !FMath::IsFinite(BonusMaxSpeed + Amount))
    {
        return false;
    }
    BonusMaxSpeed += Amount;
    NotifyChanged();
    ForceNetUpdate();
    return true;
}

/// 두 잔액을 먼저 검증하므로 실패한 거래에는 부분 지급이 없습니다.
bool ACXPlayerState::TransferCoinsTo(ACXPlayerState* Receiver, int32 Amount)
{
    if (!HasAuthority() || !IsValid(Receiver) || !Receiver->HasAuthority() || Receiver == this ||
        Receiver->GetWorld() != GetWorld() || Amount <= 0 || Coins < Amount ||
        Receiver->Coins > MAX_int32 - Amount)
    {
        return false;
    }
    Coins -= Amount;
    Receiver->Coins += Amount;
    NotifyChanged();
    Receiver->NotifyChanged();
    ForceNetUpdate();
    Receiver->ForceNetUpdate();
    return true;
}

/// 복제 수신과 리슨 서버의 상태 변경을 같은 알림으로 처리합니다.
void ACXPlayerState::NotifyChanged()
{
    OnChanged.Broadcast();
}
