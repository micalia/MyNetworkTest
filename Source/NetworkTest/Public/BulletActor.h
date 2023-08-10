// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletActor.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE (FTestSignature);

UCLASS()
class NETWORKTEST_API ABulletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = MySettings)
	class USphereComponent* sphereComp;

	UPROPERTY(VisibleAnywhere, Category = MySettings)
	class UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, Category = MySettings)
	class UProjectileMovementComponent* projectileMovement;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FTestSignature testDele;
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TestFunc();




private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
