#include "ChatUI.h"
#include "CXPlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/MultiLineEditableTextBox.h"

/// 위젯 재구성 시에도 이벤트는 한 번만 연결합니다.
void UChatUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (IsValid(ChatInput))
    {
        ChatInput->OnTextCommitted.AddUniqueDynamic(this, &UChatUI::OnChatInputTextCommitted);
    }
}
/// 화면 제거 시 입력 이벤트를 해제합니다.
void UChatUI::NativeDestruct()
{
    if (IsValid(ChatInput))
    {
        ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommitted);
    }
    Super::NativeDestruct();
}
/// 로컬 입력을 검증하고 서버 요청 후 게임 입력으로 복귀합니다.
void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod != ETextCommit::OnEnter || !IsValid(ChatInput))
    {
        return;
    }
    ACXPlayerController* PlayerController = GetOwningPlayer<ACXPlayerController>();
    if (!IsValid(PlayerController))
    {
        return;
    }
    const FString Message = Text.ToString().TrimStartAndEnd();
    if (!Message.IsEmpty() && Message.Len() <= 200)
    {
        PlayerController->CTSChatMessage(Message);
    }
    ChatInput->SetText(FText::GetEmpty());
    UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
    PlayerController->bShowMouseCursor = false;
}
/// 채팅 기록의 개수를 제한하여 무제한 누적을 막습니다.
void UChatUI::OnNewChatMessage(const FString& Sender, const FString& Message)
{
    Lines.Add(Sender + TEXT(": ") + Message);
    if (Lines.Num() > 100)
    {
        Lines.RemoveAt(0, Lines.Num() - 100);
    }
    if (IsValid(ChatLog))
    {
        ChatLog->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
    }
}
/// 채팅창에 입력 초점을 부여합니다.
void UChatUI::FocusInput()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!IsValid(PlayerController) || !IsValid(ChatInput))
    {
        return;
    }
    FInputModeGameAndUI Mode;
    Mode.SetWidgetToFocus(ChatInput->TakeWidget());
    Mode.SetHideCursorDuringCapture(false);
    PlayerController->SetInputMode(Mode);
    PlayerController->bShowMouseCursor = true;
    ChatInput->SetKeyboardFocus();
}
