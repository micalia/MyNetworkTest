// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OwnerTestActor.generated.h"

UCLASS()
class NETWORKTEST_API AOwnerTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AOwnerTestActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = MySettings)
	class UStaticMeshComponent* meshComp;

private:
	enum ENetRole localRole;
	enum ENetRole remoteRole;
	float searchDistance = 500;

	void PrintLog();
};
