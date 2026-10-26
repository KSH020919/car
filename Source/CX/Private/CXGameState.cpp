#include "CXGameState.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

/// 기존 프로젝트에 포함된 연출 애셋을 기본값으로 연결합니다.
ACXGameState::ACXGameState()
{
    static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(
        TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
    static ConstructorHelpers::FObjectFinder<USoundBase> Sound(
        TEXT("/Game/StarterContent/Audio/Explosion01.Explosion01"));
    Effect = Particle.Object;
    PickupSound = Sound.Object;
}

/// 게임 진행 결과를 모든 참가자에게 전달합니다.
void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACXGameState, Phase);
    DOREPLIFETIME(ACXGameState, RemainingSeconds);
    DOREPLIFETIME(ACXGameState, WinnerIds);
    DOREPLIFETIME(ACXGameState, WinnerNames);
}

/// 서버의 단계와 시간을 바꾸고 호스트 화면에도 알립니다.
void ACXGameState::SetRound(ECXRoundPhase NewPhase, int32 Seconds)
{
    if (!HasAuthority())
    {
        return;
    }
    Phase = NewPhase;
    RemainingSeconds = FMath::Max(Seconds, 0);
    NotifyChanged();
    ForceNetUpdate();
}

/// 종료 시점 이름과 고유 식별자를 함께 보관합니다.
void ACXGameState::SetWinners(const TArray<int32>& WinnerPlayerIds, const TArray<FString>& WinnerPlayerNames)
{
    if (!HasAuthority())
    {
        return;
    }
    WinnerIds = WinnerPlayerIds;
    WinnerNames = WinnerPlayerNames;
    NotifyChanged();
    ForceNetUpdate();
}

/// 공용 상태 갱신을 구독한 화면에 알립니다.
void ACXGameState::NotifyChanged()
{
    OnChanged.Broadcast();
}

/// 파괴되는 아이템 대신 살아 있는 게임 상태에서 일회성 연출을 전달합니다.
void ACXGameState::MulticastEffect_Implementation(FVector Location, bool bPickup)
{
    if (GetNetMode() == NM_DedicatedServer || !IsValid(GetWorld()))
    {
        return;
    }
    if (IsValid(Effect))
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effect, Location, FRotator::ZeroRotator,
                                                 FVector(0.25f));
    }
    if (bPickup && IsValid(PickupSound))
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Location, 0.3f);
    }
}

/// 서버와 클라이언트가 같은 추적 시작점과 끝점을 확인하도록 표시합니다.
void ACXGameState::MulticastAttackLine_Implementation(FVector Start, FVector End, bool bHit)
{
    if (IsValid(GetWorld()))
    {
        DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 3.0f);
    }
}
