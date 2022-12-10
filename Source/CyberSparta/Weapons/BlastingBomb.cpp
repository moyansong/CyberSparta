// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastingBomb.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"

ABlastingBomb::ABlastingBomb()
{
	Ammo = 1;
	MaxAmmo = 1;
	AmmoCostPerFire = 0;
	bCanReload = false;
	bCanAutomaticFire = false;
	Team = ETeam::ET_RedTeam;
}

void ABlastingBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABlastingBomb::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter && OtherCharacter->GetTeam() == Team)
	{
		OtherCharacter->SetOverlappingActor(this);
	}
}

void ABlastingBomb::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter && OtherCharacter->GetTeam() == Team)
	{
		OtherCharacter->SetOverlappingActor(nullptr);
	}
}

void ABlastingBomb::InteractStart(AActor* InteractActor)
{
	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		
	}
}

void ABlastingBomb::InteractStop(AActor* InteractActor)
{
	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter && InteractCharacter->GetTeam() == Team && Team == ETeam::ET_RedTeam)
	{
		Pickup(InteractCharacter);
	}
}
