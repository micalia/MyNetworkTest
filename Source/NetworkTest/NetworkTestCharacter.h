// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NetworkTestCharacter.generated.h"


UCLASS(config=Game)
class ANetworkTestCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* releaseWeapon;

public:
	ANetworkTestCharacter();
	

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
			

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE void SetOwningWeapon(class APistolActor* pistol) { owningWeapon = pistol; }
	FORCEINLINE class APistolActor* GetOwningWeapon() const { return owningWeapon; }
	FORCEINLINE int32 GetAmmo() const { return ammo; }
	FORCEINLINE void SetAmmo(int32 count) { ammo = count; }
	FORCEINLINE void SetAttckPower(int32 damage) { attackPower = damage; }
	FORCEINLINE void SetFireInterval(float time) { fireInterval = time; }


	UPROPERTY(EditAnywhere, Category = MySettings)
	TSubclassOf<class UBattleWidget> battleWidget;
	class UBattleWidget* battle_UI;

private:
	enum ENetRole myLocalRole;
	enum ENetRole myRemoteRole;
	UPROPERTY(Replicated)
	float timeTest = 0;

	UPROPERTY(ReplicatedUsing=OnRep_JumpNotify)
	int32 jumpCount = 0;

	UFUNCTION()
	void OnRep_JumpNotify();

	void PrintLog();
	void OnJump();
	void ReleaseWeapon();

	UFUNCTION(Server, Reliable)
	void ServerOnJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnJump();

	UPROPERTY(Replicated)
	class APistolActor* owningWeapon;

	int32 ammo = 10;
	int32 attackPower = 2;
	float fireInterval = 0.2f;
};

