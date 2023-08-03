// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "NetGameInstance.h"
#include "Components/WidgetSwitcher.h"

void ULoginWidget::NativeConstruct()
{
	gi = GetGameInstance<UNetGameInstance>();

	btn_CreateSession->OnClicked.AddDynamic(this, &ULoginWidget::OnClickCreateButton);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnSliderMoved);
	btn_CreateSelection->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedCreateSelection);
	btn_FindSelection->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedFindSelection);
	btn_FindSession->OnClicked.AddDynamic(this, &ULoginWidget::OnClickFindButton);
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

void ULoginWidget::SwitchCanvas(int32 index)
{
	ws_SessionUISwitch->SetActiveWidgetIndex(index);
}

