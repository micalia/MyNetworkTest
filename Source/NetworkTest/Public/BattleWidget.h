// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKTEST_API UBattleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = MySettings)
	class UTextBlock* text_Ammo = 0;

	UPROPERTY(VisibleAnywhere, meta = (BindWidgetAnim), Transient, Category = MySettings)
	class UWidgetAnimation* hitAnim;

	void PlayHitAnim();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

private:
	class ANetworkTestCharacter* player;
};
