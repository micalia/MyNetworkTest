// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleWidget.h"
#include "../NetworkTestCharacter.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "NetGameInstance.h"

void UBattleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	player = GetOwningPlayerPawn<ANetworkTestCharacter>();
	btn_ExitSession->OnClicked.AddDynamic(this, &UBattleWidget::OnExitSession);
}

void UBattleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 플레이어의 총알 정보 갱신
	if (player != nullptr)
	{
		text_Ammo->SetText(FText::AsNumber(player->GetAmmo()));
	}
}

void UBattleWidget::OnExitSession()
{
	if (player->GetController() != nullptr && player->GetController()->IsLocalPlayerController())
	{
		UNetGameInstance* gi = GetGameInstance<UNetGameInstance>();
		if (gi != nullptr)
		{
			gi->sessionInterface->DestroySession(FName(gi->mySessionName));
			player->GetController<APlayerController>()->ClientTravel("/Game/Maps/LobbyMap", ETravelType::TRAVEL_Absolute);
		}
	}
	player->GetController<APlayerController>()->SetInputMode(FInputModeGameAndUI());
}

void UBattleWidget::PlayHitAnim()
{
	PlayAnimationForward(hitAnim);
}
