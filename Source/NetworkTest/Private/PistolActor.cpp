// Fill out your copyright notice in the Description page of Project Settings.


#include "PistolActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "../NetworkTestCharacter.h"
#include "PlayerAnimInstance.h"
#include "BulletActor.h"


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
	boxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	boxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>("Static Mesh");
	meshComp->SetupAttachment(RootComponent);
	meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bReplicates = true;
	SetReplicateMovement(true);
}

void APistolActor::BeginPlay()
{
	Super::BeginPlay();
	boxComp->OnComponentBeginOverlap.AddDynamic(this, &APistolActor::OnOverlap);
}

void APistolActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APistolActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ANetworkTestCharacter* player = Cast<ANetworkTestCharacter>(OtherActor);

	if (player != nullptr && player->GetOwningWeapon() == nullptr)
	{
		SetOwner(player);

		// 서버에 총을 잡는 기능 수행을 요청한다.
		ServerGrabWeapon(player);
	}
}

void APistolActor::ServerGrabWeapon_Implementation(ANetworkTestCharacter* player)
{
	player->SetOwningWeapon(this);

	// 총의 정보 넘기기
	player->SetAmmo(ammo);
	player->SetAttckPower(attackPower);
	player->SetFireInterval(fireInterval);

	MulticastGrabWeapon(player);
	UE_LOG(LogTemp, Warning, TEXT("SeverGrabWeapon Call!"));
}

void APistolActor::MulticastGrabWeapon_Implementation(ANetworkTestCharacter* player)
{
	UE_LOG(LogTemp, Warning, TEXT("NetMulticastGrabWeapon Call!"));
	// 무기를 장착한다.
	boxComp->SetSimulatePhysics(false);
	FAttachmentTransformRules rules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;

	if (player != nullptr)
	{
		AttachToComponent(player->GetMesh(), rules, FName("Pistol Loc"));

		boxComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);

		// 애니메이션 변경하기
		UPlayerAnimInstance* anim = Cast<UPlayerAnimInstance>(player->GetMesh()->GetAnimInstance());

		if (anim != nullptr)
		{
			anim->bHasPistol = true;
		}
	}
}

void APistolActor::ServerFireBullet_Implementation(ANetworkTestCharacter* player)
{
	if (bullet != nullptr)
	{
		if (player->GetAmmo() > 0)
		{
			FActorSpawnParameters params;
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ABulletActor* spawnedBullet = GetWorld()->SpawnActor<ABulletActor>(bullet, meshComp->GetSocketLocation(FName("Fire Location")), meshComp->GetSocketRotation(FName("Fire Location")), params);

			spawnedBullet->SetOwner(player);
			player->SetAmmo(player->GetAmmo() - 1);
			player->MulticastFire(true);
		}
		else
		{
			player->MulticastFire(false);
		}
	}
}


void APistolActor::ServerReleaseWeapon_Implementation(class ANetworkTestCharacter* player)
{
	ammo = player->GetAmmo();
	player->WeaponInfoReset();
	MulticastReleaseWeapon(player);
	player->SetOwningWeapon(nullptr);
}

void APistolActor::MulticastReleaseWeapon_Implementation(class ANetworkTestCharacter* player)
{
	FDetachmentTransformRules rules = FDetachmentTransformRules::KeepWorldTransform;
	DetachFromActor(rules);
	boxComp->SetSimulatePhysics(true);

	FTimerHandle colTimer;
	GetWorldTimerManager().SetTimer(colTimer, FTimerDelegate::CreateLambda([&]() {
		boxComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
		}), 3.0f, false);

	if (UPlayerAnimInstance* anim = Cast<UPlayerAnimInstance>(player->GetMesh()->GetAnimInstance()))
	{
		anim->bHasPistol = false;
	}

	if (HasAuthority())
	{
		SetOwner(nullptr);
	}
}

void APistolActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APistolActor, ammo);
}