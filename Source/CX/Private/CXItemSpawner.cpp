#include "CXItemSpawner.h"
#include "CXPickup.h"
#include "CXSpeedBuff.h"
#include "CXGameState.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

/// 편집 가능한 위치 기준과 기본 버프 종류를 준비합니다.
ACXItemSpawner::ACXItemSpawner()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot")));
    ItemClass = ACXSpeedBuff::StaticClass();
}

/// 각 클라이언트에서 독립적인 아이템을 만들지 않도록 서버만 실행합니다.
void ACXItemSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        SpawnItem();
    }
}

/// 종료 중에는 파괴 알림이 리스폰을 예약하지 않도록 해제합니다.
void ACXItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    bStopping = true;
    if (IsValid(GetWorld()))
    {
        GetWorldTimerManager().ClearTimer(RespawnTimer);
    }
    if (IsValid(SpawnedItem))
    {
        SpawnedItem->OnDestroyed.RemoveDynamic(this, &ACXItemSpawner::HandleItemDestroyed);
    }
    Super::EndPlay(EndPlayReason);
}

/// 같은 클래스의 새 아이템을 만들고 새 객체의 파괴 알림을 구독합니다.
void ACXItemSpawner::SpawnItem()
{
    if (!HasAuthority() || bStopping || !IsValid(GetWorld()) || !ItemClass || IsValid(SpawnedItem))
    {
        return;
    }
    ACXGameState* GameState = GetWorld()->GetGameState<ACXGameState>();
    if (IsValid(GameState) && GameState->GetPhase() == ECXRoundPhase::Finished)
    {
        return;
    }
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnedItem = GetWorld()->SpawnActor<ACXPickup>(ItemClass, GetActorTransform(), Params);
    if (IsValid(SpawnedItem))
    {
        SpawnedItem->OnDestroyed.AddUniqueDynamic(this, &ACXItemSpawner::HandleItemDestroyed);
    }
}

/// 파괴된 객체를 비우고 정확히 하나의 재생성 타이머를 예약합니다.
void ACXItemSpawner::HandleItemDestroyed(AActor* DestroyedActor)
{
    if (!HasAuthority() || bStopping || !IsValid(GetWorld()) || DestroyedActor != SpawnedItem)
    {
        return;
    }
    SpawnedItem = nullptr;
    GetWorldTimerManager().SetTimer(RespawnTimer, this, &ACXItemSpawner::SpawnItem,
                                    FMath::Max(0.1f, RespawnDelay), false);
}
