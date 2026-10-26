#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXPickup.generated.h"

class ACXPlayerState;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

/// 서버에서 한 번만 획득 처리하는 복제 아이템의 공통 기반입니다.
UCLASS(Abstract)
class CX_API ACXPickup : public AActor
{
    GENERATED_BODY()
  public:
    /// 오버랩 영역과 표시용 메시를 만들고 복제 주기를 설정합니다.
    ACXPickup();

  protected:
    /// 서버에서만 획득 오버랩을 구독합니다.
    virtual void BeginPlay() override;
    /// 유효한 차량의 개인 상태에 아이템 보상을 지급합니다.
    virtual bool Grant(ACXPlayerState* State);
    /// 획득 가능한 차량을 감지하는 영역입니다.
    UPROPERTY(VisibleAnywhere, Category = "아이템")
    TObjectPtr<USphereComponent> PickupCollision;
    /// 모든 참가자가 볼 아이템 메시입니다.
    UPROPERTY(VisibleAnywhere, Category = "아이템")
    TObjectPtr<UStaticMeshComponent> ItemMesh;

  private:
    /// 중복 오버랩으로 보상이 두 번 지급되는 것을 방지합니다.
    bool bConsumed = false;
    /// 진행 중인 서버에서 차량을 검증하고 보상 후 아이템을 파괴합니다.
    UFUNCTION()
    void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                         UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
                         const FHitResult& SweepResult);
};
