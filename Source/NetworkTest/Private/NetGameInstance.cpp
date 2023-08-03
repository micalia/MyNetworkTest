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
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindOtherSessions);
	}
}

void UNetGameInstance::CreateMySession(FText roomName, int32 playerCount)
{
	FOnlineSessionSettings sessionSettings;

	// 1. LAN �������� DEDICATED �������� �����Ѵ�.
	sessionSettings.bIsDedicated = false;
	sessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 2. �˻� ������ ������ �����Ѵ�.
	sessionSettings.bShouldAdvertise = true;

	// 3. �ڱ� ������ ���޵� �� �ְ� �����Ѵ�.
	sessionSettings.bUsesPresence = true;
	
	// 4. �ٸ� ������ �߰� ������ ����Ѵ�.
	sessionSettings.bAllowJoinInProgress = true;
	sessionSettings.bAllowJoinViaPresence = true;

	// 5. ���� ���� �ο��� �����Ѵ�.
	sessionSettings.NumPublicConnections = playerCount;

	// 6. ���ǿ� �߰� ������ �ִ´�.
	sessionSettings.Set(FName("ROOM_NAME"), roomName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	bool isSuccess = sessionInterface->CreateSession(0, FName(roomName.ToString()), sessionSettings);
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

void UNetGameInstance::OnFindOtherSessions(bool bWasSuccessful)
{

	if (bWasSuccessful)
	{
		// �˻��� ���� ����� �����´�.
		TArray<FOnlineSessionSearchResult> searchResults = sessionSearch->SearchResults;

		UE_LOG(LogTemp, Warning, TEXT("Find Sessions: %d"), searchResults.Num());

		for (FOnlineSessionSearchResult result : searchResults)
		{
			FString roomName;
			result.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
			FString hostName;
			result.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
			int32 openNumber = result.Session.NumOpenPublicConnections;
			int32 maxNumber = result.Session.SessionSettings.NumPublicConnections;
			int32 pingSpeed = result.PingInMs;

			UE_LOG(LogTemp, Warning, TEXT("Room Name: %s, HostName: %s, OpenNumber: %d, MaxNumber: %d, Ping Speed: %d"), *roomName, *hostName, openNumber, maxNumber, pingSpeed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session search failed..."));
	}
}

