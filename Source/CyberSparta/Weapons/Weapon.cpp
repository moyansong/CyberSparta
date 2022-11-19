// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "../CyberSparta.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../Characters/MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	SetRootComponent(MeshComponent);

	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetupAttachment(RootComponent);
	// 在客户端设为无碰撞，在Server端设置为可以碰撞, Server管理所有Weapond的碰撞
	SetSphereCollision(false);

	PickupWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidgetComponent->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 在客户端设为无碰撞，在Server端设置为可以碰撞, 只有Server会发生Overlap
		SetSphereCollision(true);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereBeginOverlap);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::SetSphereCollision(bool bCanOverlapWithPawn)
{
	if (!SphereComponent)
	{
		return;
	}

	if (bCanOverlapWithPawn)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* MyCharacter = Cast<AMyCharacter>(OtherActor);
	if (MyCharacter)
	{
		MyCharacter->SetOverlappingActor(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* MyCharacter = Cast<AMyCharacter>(OtherActor);
	if (MyCharacter)
	{
		MyCharacter->SetOverlappingActor(nullptr);
	}
}

void AWeapon::InteractStart(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(InteractActor);
	if (MyCharacter)
	{
		MyCharacter->EquipWeaponStart();
	}
}

void AWeapon::InteractStop(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(InteractActor);
	if (MyCharacter)
	{
		MyCharacter->EquipWeaponStop();
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	SwitchOfWeaponState();
}

void AWeapon::SwitchOfWeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		SetInteractWidgetVisibility(false);
		SetSphereCollision(false);
		break;
	}
}

void AWeapon::SetInteractWidgetVisibility(bool bVisibility)
{
	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(bVisibility);
	}
}

void AWeapon::OnRep_WeaponState()
{
	SwitchOfWeaponState();
}

void AWeapon::FireStart(const FVector& HitTarget)
{
	
}

void AWeapon::FireStop()
{
}

void AWeapon::SimulateFire()
{
	if (FireAnimation)
	{
		MeshComponent->PlayAnimation(FireAnimation, false);
	}
}

void AWeapon::EquipWeaponStart()
{
}

void AWeapon::EquipWeaponStop()
{
}

