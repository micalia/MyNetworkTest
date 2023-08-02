// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKTEST_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;


public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class UEditableText* edit_roomName;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class USlider* slider_playerCount;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class UTextBlock* text_sliderCount;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class UButton* btn_CreateSession;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class UScrollBox* sb_RoomListBox;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget), Category=MySettings)
	class UButton* btn_FindSession;

	class UNetGameInstance* gi;

	UFUNCTION()
	void OnClickCreateButton();

	UFUNCTION()
	void OnSliderMoved(float value);
};
