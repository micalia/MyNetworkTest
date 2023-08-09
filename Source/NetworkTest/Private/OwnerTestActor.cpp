// Fill out your copyright notice in the Description page of Project Settings.


#include "OwnerTestActor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "../NetworkTestCharacter.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"


AOwnerTestActor::AOwnerTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	meshComp->SetupAttachment(RootComponent);
	meshComp->SetWorldScale3D(FVector(0.5f));

	// 변수 복제 기능 사용
	bReplicates = true;
}

void AOwnerTestActor::BeginPlay()
{
	Super::BeginPlay();
	
	localRole = GetLocalRole();
	remoteRole = GetRemoteRole();
}

void AOwnerTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//PrintLog();

	// 만일, 내가 서버에 있는 액터라면...
	if (HasAuthority())
	{
		// 만일, 일정한 반경 내에 들어온 플레이어 캐릭터가 있다면...
		AActor* newOwner = nullptr;
		float minDistance = searchDistance;

		for (TActorIterator<ANetworkTestCharacter> it(GetWorld()); it; ++it)
		{
			float currentDistance = GetDistanceTo(*it);

			// 더 가까운 캐릭터를 Owner로 설정한다.
			if (currentDistance < minDistance)
			{
				newOwner = *it;
				minDistance = currentDistance;
			}
		}

		// 가장 가까운 캐릭터가 현재의 owner 아니면 owner로 갱신한다.
		if (GetOwner() != newOwner)
		{
			SetOwner(newOwner);
		}
	}

	// 반경을 시각적으로 그리기
	DrawDebugSphere(GetWorld(), GetActorLocation(), searchDistance, 30, FColor::Green);

}

void AOwnerTestActor::PrintLog()
{
	const FString localRoleString = UEnum::GetValueAsString<ENetRole>(localRole);
	const FString remoteRoleString = UEnum::GetValueAsString<ENetRole>(remoteRole);
	const FString ownerString = GetOwner() != nullptr ? GetOwner()->GetName() : FString("No Owner");
	const FString connectionString = GetNetConnection() != nullptr ? FString("Valid Connection") : FString("Invalid Connection");
	const FString printString = FString::Printf(TEXT("Local Role: %s\nRemote Role: %s\nOwner Name: %s\nNet Connection : %s"), *localRoleString, *remoteRoleString, *ownerString, *connectionString);

	DrawDebugString(GetWorld(), GetActorLocation(), printString, nullptr, FColor::Yellow, 0, true);
}

