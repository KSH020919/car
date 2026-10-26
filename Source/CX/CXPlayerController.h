#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CXPlayerController.generated.h"
class UInputMappingContext;
class ACXPawn;
class UCXUI;
class UCXCourseHUD;

/// 화면 준비 전에 수신한 채팅을 보관합니다.
struct FCXPendingChat
{
    /// 발신자 이름입니다.
    FString Sender;
    /// 메시지 내용입니다.
    FString Message;
};
/// 차량 입력과 개인 화면 및 서버 채팅을 담당합니다.
UCLASS()
class CX_API ACXPlayerController : public APlayerController
{
    GENERATED_BODY()
  public:
    /// 클라이언트의 채팅을 서버에 요청합니다.
    UFUNCTION(Server, Reliable)
    void CTSChatMessage(const FString& Message);
    /// 서버 메시지를 해당 소유 클라이언트로 전달합니다.
    UFUNCTION(Client, Reliable)
    void STCChatMessage(const FString& Sender, const FString& Message);

  protected:
    /// 로컬 플레이어의 화면만 생성합니다.
    virtual void BeginPlay() override;
    /// 차량 매핑과 화면 단축키를 연결합니다.
    virtual void SetupInputComponent() override;
    /// 화면 갱신 타이머를 정리합니다.
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    /// 준비된 개인 상태를 화면에 연결합니다.
    virtual void OnRep_PlayerState() override;
    /// 기존 차량의 입력 매핑입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> InputMappingContext;
    /// 핸들 입력 사용 여부입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    bool bUseSteeringWheelControls = false;
    /// 선택적인 핸들 입력 매핑입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> SteeringWheelInputMappingContext;
    /// 기존 차량 속도계 클래스입니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UCXUI> VehicleUIClass;
    /// 기존 속도계 위젯입니다.
    UPROPERTY(Transient)
    TObjectPtr<UCXUI> VehicleUI;
    /// 사용자가 UMG에서 직접 제작한 전체 화면 클래스를 지정합니다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UCXCourseHUD> CourseHUDClass;
    /// 채팅과 상태 및 결과 화면입니다.
    UPROPERTY(Transient)
    TObjectPtr<UCXCourseHUD> CourseHUD;

  private:
    /// 화면 생성 전 메시지를 보관합니다.
    TArray<FCXPendingChat> PendingChat;
    /// 마지막 허용 채팅의 서버 시각입니다.
    double LastChatTime = -1000.0;
    /// 가벼운 속도계 갱신을 예약합니다.
    FTimerHandle VehicleTimer;
    /// 채팅 입력창을 활성화합니다.
    void FocusChat();
    /// 마우스 입력 모드를 전환합니다.
    void ToggleMouse();
    /// 차량의 속도와 기어를 표시합니다.
    void RefreshVehicleUI();
};
