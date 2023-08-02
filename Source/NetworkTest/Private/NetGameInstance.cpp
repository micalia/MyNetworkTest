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
		// Online Session Interface API 접근용 인스턴스 가져오기
		sessionInterface = subsys->GetSessionInterface();

		// 세션 이벤트에 함수 바인딩하기
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreatedMySession);
		/*FTimerHandle createHandler;
		GetWorld()->GetTimerManager().SetTimer(createHandler, this, &UNetGameInstance::CreateMySession, 1, false);*/
	}
}

void UNetGameInstance::CreateMySession(FText roomName, int32 playerCount)
{
	FOnlineSessionSettings sessiongSettings;

	// 1. LAN 연결인지 DEDICATED 연결인지 설정한다.
	sessiongSettings.bIsDedicated = false;
	sessiongSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 2. 검색 가능한 방으로 설정한다.
	sessiongSettings.bShouldAdvertise = true;

	// 3. 자기 정보를 전달될 수 있게 설정한다.
	sessiongSettings.bUsesPresence = true;
	
	// 4. 다른 유저의 중간 입장을 허용한다.
	sessiongSettings.bAllowJoinInProgress = true;
	sessiongSettings.bAllowJoinViaPresence = true;

	// 5. 입장 가능 인원을 설정한다.
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

	// 1. 세션 검색을 LAN으로 할 것인지 여부를 설정한다.
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 2. 세션 쿼리(query) 설정한다.
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 3. 세션의 검색량을 설정한다.
	sessionSearch->MaxSearchResults = 10;

	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

