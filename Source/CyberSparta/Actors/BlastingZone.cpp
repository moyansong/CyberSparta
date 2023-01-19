// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastingZone.h"
#include "Components/SphereComponent.h"
#include "../Characters/MyCharacter.h"
#include "../Weapons/SpecialWeapons/BlastingBomb.h"

ABlastingZone::ABlastingZone()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(SphereComponent);

	SphereComponent->SetSphereRadius(400.f);
}

void ABlastingZone::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABlastingZone::OnSphereBeginOverlap);
}

void ABlastingZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABlastingZone::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
}

void ABlastingZone::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
}



