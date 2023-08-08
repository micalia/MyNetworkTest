// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkTestCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"	// 언리얼 네트워크 기능 사용을 위한 헤더
#include "DrawDebugHelpers.h"
#include "PistolActor.h"
#include "BattleWidget.h"


ANetworkTestCharacter::ANetworkTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel2);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;
}

void ANetworkTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	myLocalRole = GetLocalRole();
	myRemoteRole = GetRemoteRole();

	// 메인 위젯 생성하기
	if (GetController() != nullptr && GetController()->IsLocalPlayerController())
	{
		battle_UI = CreateWidget<UBattleWidget>(GetWorld(), battleWidget);
		if (battle_UI != nullptr)
		{
			battle_UI->AddToViewport();
		}
	}
}

void ANetworkTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 디버깅용 로그 출력
	PrintLog();

	timeTest += DeltaSeconds;
}


// 정보를 화면에 출력하는 함수
void ANetworkTestCharacter::PrintLog()
{
	//const FString localRoleString = UEnum::GetValueAsString<ENetRole>(myLocalRole);
	//const FString remoteRoleString = UEnum::GetValueAsString<ENetRole>(myRemoteRole);
	//const FString ownerString = GetOwner() != nullptr ? GetOwner()->GetName() : FString("No Owner");
	//const FString connectionString = GetNetConnection() != nullptr ? FString("Valid Connection") : FString("Invalid Connection");
	//const FString printString = FString::Printf(TEXT("Local Role: %s\nRemote Role: %s\nOwner Name: %s\nNet Connection : %s"), *localRoleString, *remoteRoleString, *ownerString, *connectionString);

	const FString printString = FString::Printf(TEXT("Time: %.2f\nJump Count: %d"), timeTest, jumpCount);
	DrawDebugString(GetWorld(), GetActorLocation(), printString, nullptr, FColor::White, 0, true);
}

void ANetworkTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ANetworkTestCharacter::OnJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkTestCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkTestCharacter::Look);
		EnhancedInputComponent->BindAction(releaseWeapon, ETriggerEvent::Started, this, &ANetworkTestCharacter::ReleaseWeapon);
		EnhancedInputComponent->BindAction(fire, ETriggerEvent::Started, this, &ANetworkTestCharacter::Fire);
	}

}

void ANetworkTestCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetworkTestCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// 점프 키를 눌렀을 때 실행될 함수
void ANetworkTestCharacter::OnJump()
{
	ServerOnJump();
}

// 'G' 키를 눌렀을 때 실행될 함수
void ANetworkTestCharacter::ReleaseWeapon()
{
	if (owningWeapon != nullptr)
	{
		owningWeapon->ServerReleaseWeapon(this);
	}
}

void ANetworkTestCharacter::ServerOnJump_Implementation()
{
	jumpCount++;
}

void ANetworkTestCharacter::MulticastOnJump_Implementation()
{
	Jump();
}

void ANetworkTestCharacter::WeaponInfoReset()
{
	ammo = 0;
	fireInterval = 0;
	attackPower = 0;
}

void ANetworkTestCharacter::Fire()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire!"));

	FVector startLoc = owningWeapon->meshComp->GetSocketLocation(FName("Fire Location"));
	FVector endLoc = startLoc + FRotationMatrix(owningWeapon->meshComp->GetSocketRotation(FName("Fire Location"))).GetUnitAxis(EAxis::X) * 1000.0f;
	FHitResult hitInfo;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, params))
	{
		if (hitInfo.GetActor()->IsA<ANetworkTestCharacter>())
		{
			/*if (HasAuthority())
			{
				ServerFire_Implementation();
			}
			else
			{*/
				ServerFire();
			//}
		}
		UE_LOG(LogTemp, Warning, TEXT("Hit!"));
	}

	
}

void ANetworkTestCharacter::ServerFire_Implementation()
{
	ammo--;

	// 데미지 처리
}

bool ANetworkTestCharacter::ServerFire_Validate()
{
	return true;
}

// jumpCount 값이 동기화로 인하여 변경될 때 실행되는 함수
void ANetworkTestCharacter::OnRep_JumpNotify()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("%s is Jump %d times"), *GetName(), jumpCount));
}


void ANetworkTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetworkTestCharacter, timeTest);
	//DOREPLIFETIME_CONDITION(ANetworkTestCharacter, timeTest, COND_OwnerOnly);
	DOREPLIFETIME(ANetworkTestCharacter, jumpCount);
	DOREPLIFETIME(ANetworkTestCharacter, owningWeapon);
	DOREPLIFETIME(ANetworkTestCharacter, ammo);
	DOREPLIFETIME(ANetworkTestCharacter, attackPower);
	DOREPLIFETIME(ANetworkTestCharacter, fireInterval);
}

