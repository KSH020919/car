#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "ChatUI.generated.h"
class UMultiLineEditableTextBox;

/// 사용자가 만든 블루프린트에서 채팅 입력과 기록을 제공합니다.
UCLASS()
class CX_API UChatUI : public UUserWidget
{
    GENERATED_BODY()
  public:
    /// 받은 메시지를 최대 백 줄까지 표시합니다.
    void OnNewChatMessage(const FString& Sender, const FString& Message);
    /// 채팅 입력으로 초점을 옮깁니다.
    void FocusInput();

  protected:
    /// 입력 확정 이벤트를 연결합니다.
    virtual void NativeConstruct() override;
    /// 입력 확정 이벤트를 해제합니다.
    virtual void NativeDestruct() override;
    /// 기존 블루프린트의 입력창입니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UEditableTextBox> ChatInput;
    /// 기존 블루프린트의 기록창입니다.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMultiLineEditableTextBox> ChatLog;

  private:
    /// 화면에 표시할 제한된 메시지 목록입니다.
    TArray<FString> Lines;
    /// 엔터 입력을 전송한 뒤 차량 조작으로 복귀합니다.
    UFUNCTION()
    void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
