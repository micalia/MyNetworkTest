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
#include "Components/WidgetComponent.h"
#include "PlayerInfoWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"


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

	infoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Player Info Widget"));
	infoWidget->SetupAttachment(GetMesh());

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

	if (HasAuthority())
	{
		health = maxHealth;
	}

	if (infoWidget != nullptr)
	{
		info_UI = Cast<UPlayerInfoWidget>(infoWidget->GetWidget());
	}

	// 타임 라인 등록하기
	FOnTimelineFloat onProgressDash;
	onProgressDash.BindUFunction(this, FName("OnDash"));
	FOnTimelineEvent onFinishDash;
	onFinishDash.BindUFunction(this, FName("FinishDash"));

	dashTimeline.AddInterpFloat(dashCurve, onProgressDash);
	dashTimeline.SetTimelineFinishedFunc(onFinishDash);
	dashTimeline.SetTimelineLength(1.0f);
}

void ANetworkTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	dashTimeline.TickTimeline(DeltaSeconds);

	// 디버깅용 로그 출력
	PrintLog();

	timeTest += DeltaSeconds;

	if (info_UI != nullptr)
	{
		info_UI->pb_healthBar->SetPercent((float)health / (float)maxHealth);
	}

	if (health <= 0 && !bIsDead)
	{
		bIsDead = true;
		ReleaseWeapon();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
		DieProcess();
	}
}


// 정보를 화면에 출력하는 함수
void ANetworkTestCharacter::PrintLog()
{
	//const FString localRoleString = UEnum::GetValueAsString<ENetRole>(myLocalRole);
	//const FString remoteRoleString = UEnum::GetValueAsString<ENetRole>(myRemoteRole);
	//const FString ownerString = GetOwner() != nullptr ? GetOwner()->GetName() : FString("No Owner");
	//const FString connectionString = GetNetConnection() != nullptr ? FString("Valid Connection") : FString("Invalid Connection");
	//const FString printString = FString::Printf(TEXT("Local Role: %s\nRemote Role: %s\nOwner Name: %s\nNet Connection : %s"), *localRoleString, *remoteRoleString, *ownerString, *connectionString);

	const FString GameModeBaseString = GetWorld()->GetAuthGameMode() != nullptr ? *FString("Valid") : *FString("Invalid");
	const FString GameStateBaseString = GetWorld()->GetGameState() != nullptr ? *FString("Valid") : *FString("Invalid");
	const FString PlayerStateString = GetPlayerState() != nullptr ? *FString("Valid") : *FString("Invalid");

	APlayerController* pc = GetController<APlayerController>();
	const FString PlayerControllerString = pc != nullptr ? *FString("Valid") : *FString("Invalid");
	AHUD* hud = pc != nullptr ? pc->GetHUD() : nullptr;
	const FString HUDString = hud != nullptr ? *FString("Valid") : *FString("Invalid");

	const FString printString = FString::Printf(TEXT("GameModeBase: %s\nGameStateBase: %s\nPlayerState: %s\nPlayerController: %s\nHUD: %s"), *GameModeBaseString, *GameStateBaseString, *PlayerStateString, *PlayerControllerString, *HUDString);
	DrawDebugString(GetWorld(), GetActorLocation(), printString, nullptr, FColor::White, 0, true);
}

void ANetworkTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ANetworkTestCharacter::ServerStartDash);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkTestCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkTestCharacter::Look);
		EnhancedInputComponent->BindAction(releaseWeapon, ETriggerEvent::Started, this, &ANetworkTestCharacter::ReleaseWeapon);
		EnhancedInputComponent->BindAction(fire, ETriggerEvent::Started, this, &ANetworkTestCharacter::Fire);
		EnhancedInputComponent->BindAction(fire2, ETriggerEvent::Started, this, &ANetworkTestCharacter::FireType2);
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
	if (owningWeapon == nullptr)
		return;

	// RPC 오버 헤드 방지
	if (HasAuthority())
	{
		ServerFire_Implementation();
	}
	else
	{
		ServerFire();
	}
}

void ANetworkTestCharacter::FireType2()
{
	if (owningWeapon != nullptr && !bIsDead)
	{
		owningWeapon->ServerFireBullet(this);
	}
}

void ANetworkTestCharacter::ChangeView()
{
	AActor* testCam = nullptr;

	for (TActorIterator<AActor> it(GetWorld()); it; ++it)
	{
		AActor* cam = *it;
		if (cam->GetName().Contains("CameraActor"))
		{
			testCam = cam;
			
		}

		UE_LOG(LogTemp, Warning, TEXT("Find: %s"), *cam->GetName());
	}

	if (testCam != nullptr)
	{
		GetController<APlayerController>()->SetViewTargetWithBlend(testCam, 2.);
	}

}


// 체력 회복 함수
void ANetworkTestCharacter::ServerAddHealth_Implementation(int32 value)
{
	health = FMath::Min(health + value, maxHealth);
}

// 체력 감소(데미지) 함수
void ANetworkTestCharacter::ServerDamagedHealth_Implementation(int32 value)
{
	health = FMath::Max(health - value, 0);
	UE_LOG(LogTemp, Warning, TEXT("My Health is %d"), health);
}

void ANetworkTestCharacter::ServerFire_Implementation()
{
	if (GetWorldTimerManager().IsTimerActive(fireDelay))
	{
		return;
	}


	if (ammo > 0)
	{
		ammo--;

		FVector startLoc = owningWeapon->meshComp->GetSocketLocation(FName("Fire Location"));
		FVector endLoc = startLoc + FRotationMatrix(owningWeapon->meshComp->GetSocketRotation(FName("Fire Location"))).GetUnitAxis(EAxis::X) * 1000.0f;
		FHitResult hitInfo;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(hitInfo, startLoc, endLoc, ECC_Visibility, params))
		{
			//if (hitInfo.GetActor()->IsA<ANetworkTestCharacter>())
			if(ANetworkTestCharacter* hitPawn = Cast<ANetworkTestCharacter>(hitInfo.GetActor()))
			{
				// 데미지 처리
				hitPawn->ServerDamagedHealth(attackPower);
				hitPawn->ServerHitProcess();
			}
		}

		
		GetWorldTimerManager().SetTimer(fireDelay, FTimerDelegate::CreateLambda([&]() {
			bInDelay = !bInDelay;
			}), fireInterval, false);

		MulticastFire(true);
	}
	else
	{
		MulticastFire(false);
	}
}

bool ANetworkTestCharacter::ServerFire_Validate()
{
	return true;
}

void ANetworkTestCharacter::MulticastFire_Implementation(bool hasAmmo)
{
	PlayAnimMontage(fire_montages[(int)hasAmmo]);
}

void ANetworkTestCharacter::ServerHitProcess_Implementation()
{
	MulticastHitProcess();
}

void ANetworkTestCharacter::MulticastHitProcess_Implementation()
{
	APlayerController* pc = GetController<APlayerController>();

	if (pc != nullptr && pc->IsLocalPlayerController() && battle_UI != nullptr)
	{
		battle_UI->PlayHitAnim();
		pc->ClientStartCameraShake(hitShake);
	}
}

void ANetworkTestCharacter::DieProcess()
{
	if (GetController() != nullptr && GetController()->IsLocalPlayerController() && battle_UI != nullptr)
	{
		battle_UI->btn_ExitSession->SetVisibility(ESlateVisibility::Visible);
		GetController<APlayerController>()->SetShowMouseCursor(true);
		GetController<APlayerController>()->SetInputMode(FInputModeUIOnly());
		FollowCamera->PostProcessSettings.ColorSaturation = FVector4(0, 0, 0, 1);
	}
}


void ANetworkTestCharacter::ServerStartDash_Implementation()
{
	if (GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		currentDir = GetActorForwardVector();
	}
	else
	{
		currentDir = GetCharacterMovement()->Velocity;
	}
	dashTimeline.PlayFromStart();
}

void ANetworkTestCharacter::ServerEndDash_Implementation()
{
	MulticastEndDash();
}

void ANetworkTestCharacter::MulticastEndDash_Implementation()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), smokeFX, GetActorLocation() - FVector(0, 0, 30), FRotator::ZeroRotator, true);
	//GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Cyan, FString::Printf(TEXT("Finish Event!")));
}

// 타임라인이 진행 중일 때 매 프레임마다 실행되는 함수
void ANetworkTestCharacter::OnDash(float Output)
{
	GetCharacterMovement()->Velocity = currentDir * Output * dashPower;
}

// 타임라인이 종료될 때 1회 실행되는 함수
void ANetworkTestCharacter::FinishDash()
{
	if (HasAuthority())
	{
		ServerEndDash_Implementation();
	}
	else
	{
		ServerEndDash();
	}
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
	DOREPLIFETIME(ANetworkTestCharacter, health);
}

