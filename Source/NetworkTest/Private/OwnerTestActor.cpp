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

	// ���� ���� ��� ���
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

	// ����, ���� ������ �ִ� ���Ͷ��...
	if (HasAuthority())
	{
		// ����, ������ �ݰ� ���� ���� �÷��̾� ĳ���Ͱ� �ִٸ�...
		AActor* newOwner = nullptr;
		float minDistance = searchDistance;

		for (TActorIterator<ANetworkTestCharacter> it(GetWorld()); it; ++it)
		{
			float currentDistance = GetDistanceTo(*it);

			// �� ����� ĳ���͸� Owner�� �����Ѵ�.
			if (currentDistance < minDistance)
			{
				newOwner = *it;
				minDistance = currentDistance;
			}
		}

		// ���� ����� ĳ���Ͱ� ������ owner �ƴϸ� owner�� �����Ѵ�.
		if (GetOwner() != newOwner)
		{
			SetOwner(newOwner);
		}
	}

	// �ݰ��� �ð������� �׸���
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

