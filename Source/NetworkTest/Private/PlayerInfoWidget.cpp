// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInfoWidget.h"
#include "Components/ProgressBar.h"
#include "../NetworkTestCharacter.h"


void UPlayerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	player = Cast<ANetworkTestCharacter>(GetOwningPlayerPawn());
}

void UPlayerInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (player != nullptr)
	{
		// ���� ���� ü�� �� �ݿ�(���� ü�� / �ִ� ü��)
		pb_healthBar->SetPercent((float)player->GetHealth() / (float)player->maxHealth);
	}
}
