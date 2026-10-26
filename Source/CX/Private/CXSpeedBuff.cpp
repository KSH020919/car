#include "CXSpeedBuff.h"
#include "CXPlayerState.h"
#include "CXGameTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

/// 정육면체로 속도 버프의 형태를 구분합니다.
ACXSpeedBuff::ACXSpeedBuff()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (IsValid(ItemMesh))
    {
        ItemMesh->SetStaticMesh(Mesh.Object);
        ItemMesh->SetRelativeScale3D(FVector(0.5f));
    }
}

/// 속도 버프를 획득한 플레이어의 보너스를 십 증가시킵니다.
bool ACXSpeedBuff::Grant(ACXPlayerState* State)
{
    return IsValid(State) && State->AddBonusMaxSpeed(CXRules::SpeedBonusPerPickup);
}
