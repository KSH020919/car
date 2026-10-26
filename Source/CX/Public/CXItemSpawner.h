#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXItemSpawner.generated.h"

class ACXPickup;

/// 지정한 종류의 아이템을 서버에서 생성하고 파괴 후 재생성합니다.
UCLASS()
class CX_API ACXItemSpawner : public AActor
{
    GENERATED_BODY()
  public:
    /// 레벨에서 위치를 조절할 루트 컴포넌트를 만듭니다.
    ACXItemSpawner();
    /// 처음 스폰할 아이템 종류를 지정합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "스포너")
    TSubclassOf<ACXPickup> ItemClass;
    /// 파괴 후 같은 아이템을 재생성하기까지의 초입니다.
    UPROPERTY(EditAnywhere, Category = "스포너", meta = (ClampMin = "0.1"))
    float RespawnDelay = 2.0f;
    /// 현재 생성된 아이템을 추적하는 참조입니다.
    UPROPERTY(VisibleInstanceOnly, Category = "스포너")
    TObjectPtr<ACXPickup> SpawnedItem;

  protected:
    /// 서버에서 최초 아이템을 생성합니다.
    virtual void BeginPlay() override;
    /// 레벨 종료 시 타이머와 파괴 알림을 정리합니다.
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  private:
    /// 중복 예약 없이 재생성 시간을 관리합니다.
    FTimerHandle RespawnTimer;
    /// 종료 중 새 아이템을 만드는 것을 방지합니다.
    bool bStopping = false;
    /// 서버에서 지정된 종류를 생성하고 파괴 알림을 연결합니다.
    void SpawnItem();
    /// 아이템 파괴 후 같은 종류의 재생성을 예약합니다.
    UFUNCTION()
    void HandleItemDestroyed(AActor* DestroyedActor);
};
