/// 원본 저작권은 Epic Games, Inc.에 있으며 모든 권리를 보유합니다.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CXPawn.generated.h"

class UInputMappingContext;
class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/// 차량 입력과 카메라 및 서버 공격을 관리합니다.
UCLASS(abstract)
class ACXPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

    /// 전방 카메라의 스프링 암입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* FrontSpringArm;

    /// 전방 카메라 컴포넌트입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FrontCamera;

    /// 후방 카메라의 스프링 암입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* BackSpringArm;

    /// 후방 카메라 컴포넌트입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* BackCamera;

    /// 차량 이동 컴포넌트의 수명을 추적합니다.
    UPROPERTY(Transient)
    TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

  protected:
    /// 차량 조향 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* SteeringAction;

    /// 차량 가속 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ThrottleAction;

    /// 차량 제동 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* BrakeAction;

    /// 차량 주차 브레이크 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HandbrakeAction;

    /// 카메라 둘러보기 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAroundAction;

    /// 카메라 전환 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ToggleCameraAction;

    /// 차량 복구 입력입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ResetVehicleAction;

    /// 활성화된 카메라를 기록합니다.
    bool bFrontCameraActive = false;

  public:
    /// 서버에서 공격을 검증합니다.
    UFUNCTION(Server, Reliable)
    void CTSAttack();
    /// 원화 이전에 성공한 공격만 처리합니다.
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;
    /// 앞 범퍼 중앙의 서버 추적 기준점입니다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "공격")
    TObjectPtr<USceneComponent> FrontBumper;
    /// 좌클릭 공격을 나타내는 입력 액션입니다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "공격")
    TObjectPtr<UInputAction> AttackAction;
    /// 차량의 공격 매핑 수명을 유지합니다.
    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> AttackMapping;
    /// 연속 공격을 제한하는 서버 간격입니다.
    UPROPERTY(EditDefaultsOnly, Category = "공격", meta = (ClampMin = "0.1"))
    float AttackCooldown = 0.5f;
    /// 범퍼에서 전방으로 검사할 거리입니다.
    UPROPERTY(EditDefaultsOnly, Category = "공격", meta = (ClampMin = "100"))
    float AttackRange = 700.0f;
    /// 마지막 허용 공격의 서버 시각입니다.
    double LastAttackTime = -1000.0;
    /// 로컬 입력에서 서버 공격만 요청합니다.
    void InputAttack();
    /// 기존 차량과 앞 범퍼 기준점을 만듭니다.
    ACXPawn();

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.

    /// 차량과 공격 입력을 입력 컴포넌트에 연결합니다.
    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.

    /// 가벼운 카메라 복귀와 공중 감쇠만 갱신합니다.
    virtual void Tick(float Delta) override;

    /// 기존 차량 템플릿의 입력과 카메라 동작을 유지합니다.

  protected:
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void Steering(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void Throttle(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void Brake(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void StartBrake(const FInputActionValue& Value);
    void StopBrake(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void StartHandbrake(const FInputActionValue& Value);
    void StopHandbrake(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void LookAround(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void ToggleCamera(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    void ResetVehicle(const FInputActionValue& Value);

    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
    void BrakeLights(bool bBraking);

  public:
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    /// 전방 카메라의 스프링 암을 반환합니다.
    FORCEINLINE USpringArmComponent* GetFrontSpringArm() const
    {
        return FrontSpringArm;
    }
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    /// 전방 카메라를 반환합니다.
    FORCEINLINE UCameraComponent* GetFollowCamera() const
    {
        return FrontCamera;
    }
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    /// 후방 카메라의 스프링 암을 반환합니다.
    FORCEINLINE USpringArmComponent* GetBackSpringArm() const
    {
        return BackSpringArm;
    }
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    /// 후방 카메라를 반환합니다.
    FORCEINLINE UCameraComponent* GetBackCamera() const
    {
        return BackCamera;
    }
    /// 차량 기능의 상태와 호출 인터페이스를 제공합니다.
    /// 차량 이동 컴포넌트 참조를 반환합니다.
    FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const
    {
        return ChaosVehicleMovement;
    }
};
