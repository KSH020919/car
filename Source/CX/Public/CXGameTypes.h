#pragma once
#include "CoreMinimal.h"
#include "CXGameTypes.generated.h"

/// 게임의 대기와 진행 및 종료 단계를 표현합니다.
UENUM(BlueprintType)
enum class ECXRoundPhase : uint8
{
    Waiting,
    Playing,
    Finished
};

/// 화면이 상태 변경을 구독하는 알림입니다.
DECLARE_MULTICAST_DELEGATE(FCXStateChanged);

/// 게임 규칙에 쓰는 고정 수치를 한곳에서 정의합니다.
namespace CXRules
{
/// 접속한 플레이어에게 지급하는 시작 코인 수입니다.
constexpr int32 StartingCoins = 50;
/// 공격 한 번으로 상대에게서 가져오는 코인 수입니다.
constexpr int32 StealAmount = 5;
/// 버프 하나를 획득할 때 증가하는 추가 최대 속도입니다.
constexpr float SpeedBonusPerPickup = 10.0f;
} // namespace CXRules
