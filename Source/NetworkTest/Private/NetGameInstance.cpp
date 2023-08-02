// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


UNetGameInstance::UNetGameInstance()
{
	mySessionName = TEXT("WonSeok");

}

void UNetGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* subsys = IOnlineSubsystem::Get())
	{
		// Online Session Interface API ���ٿ� �ν��Ͻ� ��������
		sessionInterface = subsys->GetSessionInterface();

		// ���� �̺�Ʈ�� �Լ� ���ε��ϱ�
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreatedMySession);
		/*FTimerHandle createHandler;
		GetWorld()->GetTimerManager().SetTimer(createHandler, this, &UNetGameInstance::CreateMySession, 1, false);*/
	}
}

void UNetGameInstance::CreateMySession(FText roomName, int32 playerCount)
{
	FOnlineSessionSettings sessiongSettings;

	// 1. LAN �������� DEDICATED �������� �����Ѵ�.
	sessiongSettings.bIsDedicated = false;
	sessiongSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 2. �˻� ������ ������ �����Ѵ�.
	sessiongSettings.bShouldAdvertise = true;

	// 3. �ڱ� ������ ���޵� �� �ְ� �����Ѵ�.
	sessiongSettings.bUsesPresence = true;
	
	// 4. �ٸ� ������ �߰� ������ ����Ѵ�.
	sessiongSettings.bAllowJoinInProgress = true;
	sessiongSettings.bAllowJoinViaPresence = true;

	// 5. ���� ���� �ο��� �����Ѵ�.
	sessiongSettings.NumPublicConnections = playerCount;

	bool isSuccess = sessionInterface->CreateSession(0, FName(roomName.ToString()), sessiongSettings);
	UE_LOG(LogTemp, Warning, TEXT("Session Create Result: %s"), isSuccess ? *FString("Success") : *FString("Failed..."));


}

void UNetGameInstance::OnCreatedMySession(FName sessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		bool result = GetWorld()->ServerTravel("/Game/Maps/BattleMap?Listen", true);
		UE_LOG(LogTemp, Warning, TEXT("Travel Result: %s"), result ? *FString("Success") : *FString("Failed..."));
	}
}

void UNetGameInstance::FindOtherSession()
{
	sessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 1. ���� �˻��� LAN���� �� ������ ���θ� �����Ѵ�.
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 2. ���� ����(query) �����Ѵ�.
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 3. ������ �˻����� �����Ѵ�.
	sessionSearch->MaxSearchResults = 10;

	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

