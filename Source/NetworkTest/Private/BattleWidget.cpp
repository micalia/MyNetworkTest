// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleWidget.h"
#include "../NetworkTestCharacter.h"
#include "Components/TextBlock.h"

void UBattleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	player = GetOwningPlayerPawn<ANetworkTestCharacter>();
}

void UBattleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// �÷��̾��� �Ѿ� ���� ����
	if (player != nullptr)
	{
		text_Ammo->SetText(FText::AsNumber(player->GetAmmo()));
	}
}

void UBattleWidget::PlayHitAnim()
{
	PlayAnimationForward(hitAnim);
}
