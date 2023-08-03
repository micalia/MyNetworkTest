// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "NetGameInstance.h"
#include "Components/WidgetSwitcher.h"
#include "SessionInfoWidget.h"
#include "Components/ScrollBox.h"

void ULoginWidget::NativeConstruct()
{
	gi = GetGameInstance<UNetGameInstance>();

	btn_CreateSession->OnClicked.AddDynamic(this, &ULoginWidget::OnClickCreateButton);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnSliderMoved);
	btn_CreateSelection->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedCreateSelection);
	btn_FindSelection->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedFindSelection);
	btn_FindSession->OnClicked.AddDynamic(this, &ULoginWidget::OnClickFindButton);

	if (gi != nullptr)
	{
		gi->onSearchCompleted.AddDynamic(this, &ULoginWidget::AddRoomSlot);
	}
}

void ULoginWidget::OnClickCreateButton()
{
	if (gi != nullptr && !edit_roomName->GetText().IsEmpty())
	{
		gi->CreateMySession(edit_roomName->GetText(), (int32)slider_playerCount->GetValue());
	}
}

void ULoginWidget::OnSliderMoved(float value)
{
	text_sliderCount->SetText(FText::AsNumber((int32)value));
}

void ULoginWidget::OnClickedCreateSelection()
{
	SwitchCanvas(1);
}

void ULoginWidget::OnClickedFindSelection()
{
	SwitchCanvas(2);
}

void ULoginWidget::OnClickFindButton()
{
	if (gi != nullptr)
	{
		gi->FindOtherSession();
	}
}

void ULoginWidget::AddRoomSlot(FSessionSlotInfo slotInfo)
{
	// 슬롯 위젯을 생성한다.
	sessionSlot = CreateWidget<USessionInfoWidget>(GetWorld(), sessionInfoWidget);

	if (sessionSlot != nullptr)
	{
		// 슬롯 위젯의 각 요소에 슬롯 정보를 넣는다.
		sessionSlot->text_RoomName->SetText(FText::FromString(slotInfo.roomName));
		sessionSlot->text_HostName->SetText(FText::FromString(slotInfo.hostName));
		sessionSlot->text_PlayerCount->SetText(FText::FromString(slotInfo.playerCount));
		sessionSlot->text_PingSpeed->SetText(FText::AsNumber(slotInfo.pingSpeed));

		// 생성한 슬롯 위젯을 스크롤 박스에 자식으로 추가한다.
		sb_RoomListBox->AddChild(sessionSlot);
	}
}

void ULoginWidget::SwitchCanvas(int32 index)
{
	ws_SessionUISwitch->SetActiveWidgetIndex(index);
}

