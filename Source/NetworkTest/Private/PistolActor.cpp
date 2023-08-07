// Fill out your copyright notice in the Description page of Project Settings.


#include "PistolActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


APistolActor::APistolActor()
{
	PrimaryActorTick.bCanEverTick = true;

	boxComp = CreateDefaultSubobject<UBoxComponent>("Box Component");
	SetRootComponent(boxComp);
	boxComp->SetGenerateOverlapEvents(true);
	boxComp->SetBoxExtent(FVector(20, 10, 20));
	boxComp->SetSimulatePhysics(true);
	boxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	boxComp->SetCollisionObjectType(ECC_GameTraceChannel1);
	boxComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh");
	meshComp->SetupAttachment(RootComponent);

	bReplicates = true;
	SetReplicateMovement(true);
}

void APistolActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void APistolActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

