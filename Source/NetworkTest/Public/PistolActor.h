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

	UPROPERTY(EditDefaultsOnly, Category = MySettings)
	TSubclassOf<class ABulletActor> bullet;

	UFUNCTION(Server, Unreliable)
	void ServerFireBullet(class ANetworkTestCharacter* player);

	UFUNCTION(Server, Reliable)
	void ServerReleaseWeapon(class ANetworkTestCharacter* player);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReleaseWeapon(class ANetworkTestCharacter* player);


private:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Reliable)
	void ServerGrabWeapon(class ANetworkTestCharacter* player);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGrabWeapon(class ANetworkTestCharacter* player);

	UPROPERTY(Replicated)
	int32 ammo = 10;
	int32 attackPower = 20;
	float fireInterval = 0.5f;
};
