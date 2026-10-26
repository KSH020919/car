#pragma once
#include "CoreMinimal.h"
#include "CXPickup.h"
#include "CXSpeedBuff.generated.h"

/// 획득한 플레이어에게 속도 보너스 십을 지급합니다.
UCLASS()
class CX_API ACXSpeedBuff : public ACXPickup
{
    GENERATED_BODY()
  public:
    /// 속도 버프의 표시 크기를 설정합니다.
    ACXSpeedBuff();

  protected:
    /// 서버에서 추가 최대 속도를 한 번 증가시킵니다.
    virtual bool Grant(ACXPlayerState* State) override;
};
