#include "CXPickup.h"
#include "CXPlayerState.h"
#include "CXGameState.h"
#include "CXPawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

/// 아이템 자체는 복제하고 획득 영역은 차량 채널만 감지합니다.
ACXPickup::ACXPickup()
{
    bReplicates = true;
    SetNetUpdateFrequency(1.0f);
    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    SetRootComponent(PickupCollision);
    PickupCollision->InitSphereRadius(100.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PickupCollision->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
    PickupCollision->SetGenerateOverlapEvents(true);
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(PickupCollision);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    ItemMesh->SetStaticMesh(Mesh.Object);
}

/// 클라이언트의 오버랩으로 지급이 중복되지 않도록 서버만 구독합니다.
void ACXPickup::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && IsValid(PickupCollision))
    {
        PickupCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACXPickup::OnPickupOverlap);
    }
}

/// 공통 기반 클래스 자체는 보상을 지급하지 않습니다.
bool ACXPickup::Grant(ACXPlayerState* State)
{
    return false;
}

/// 게임 진행과 차량 및 개인 상태를 확인하고 보상을 한 번만 지급합니다.
void ACXPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
                                const FHitResult& SweepResult)
{
    if (!HasAuthority() || bConsumed || !IsValid(GetWorld()))
    {
        return;
    }
    ACXGameState* GameState = GetWorld()->GetGameState<ACXGameState>();
    ACXPawn* Vehicle = Cast<ACXPawn>(OtherActor);
    if (!IsValid(GameState) || GameState->GetPhase() != ECXRoundPhase::Playing || !IsValid(Vehicle))
    {
        return;
    }
    ACXPlayerState* PlayerState = Vehicle->GetPlayerState<ACXPlayerState>();
    if (!IsValid(PlayerState))
    {
        return;
    }
    bConsumed = true;
    if (!Grant(PlayerState))
    {
        bConsumed = false;
        return;
    }
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GameState->MulticastEffect(GetActorLocation(), true);
    Destroy();
}
