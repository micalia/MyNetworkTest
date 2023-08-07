// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PistolActor.generated.h"

UCLASS()
class NETWORKTEST_API APistolActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APistolActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category=MySettings)
	class UBoxComponent* boxComp;

	UPROPERTY(VisibleAnywhere, Category=MySettings)
	class UStaticMeshComponent* meshComp;

};
