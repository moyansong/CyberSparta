// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecialWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"

ASpecialWeapon::ASpecialWeapon()
{
	bIsRangedWeapon = false;
	bUseLeftHandIK = false;
	WeaponType = EWeaponType::EWT_SpecialWeapon;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMeshComponent);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComponent->SetRenderCustomDepth(true);
	StaticMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);

	GetSphere()->SetupAttachment(StaticMeshComponent);
	GetPickupWidget()->SetupAttachment(StaticMeshComponent);
}

void ASpecialWeapon::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	StaticMeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	MyCharacter = nullptr;
	MyController = nullptr;
	Ammo = MaxAmmo;
}

void ASpecialWeapon::Throw(FVector ThrowDirection, float Force)
{
	Drop();
	if (StaticMeshComponent)
	{
		StaticMeshComponent->AddImpulseAtLocation(ThrowDirection * Force, GetActorLocation());
	}
}

void ASpecialWeapon::OnEquipped()
{
	Super::OnEquipped();

	if (!StaticMeshComponent) return;
	StaticMeshComponent->SetVisibility(true);
	StaticMeshComponent->SetRenderCustomDepth(false);
	SetMeshSimulatePhysics(StaticMeshComponent, false);
}

void ASpecialWeapon::OnDropped()
{
	if (!GetSphere() || !StaticMeshComponent) return;

	if (HasAuthority())
	{
		GetSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	StaticMeshComponent->SetVisibility(true);
	StaticMeshComponent->SetRenderCustomDepth(true);
	SetMeshSimulatePhysics(StaticMeshComponent, true);
}

void ASpecialWeapon::OnIdled()
{
	Super::OnIdled();

	if (!StaticMeshComponent) return;
	StaticMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetRenderCustomDepth(false);
	SetMeshSimulatePhysics(StaticMeshComponent, false);
}


