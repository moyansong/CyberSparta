// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Components/RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	Damage = 100.f;
	InitialLifeSpan = 7.f;

	GetCollisionComponent()->SetBoxExtent(FVector(20.f, 10.f, 10.f));
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->SetIsReplicated(true);
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->InitialSpeed = 3000.f;
	RocketMovementComponent->MaxSpeed = 3000.f;
	RocketMovementComponent->ProjectileGravityScale = 0.f;
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AMyCharacter* HitCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherActor == GetOwner() || (HitCharacter && !HitCharacter->IsAlive()))
	{
		return;
	}

	if (bReplicates && HasAuthority())
	{
		if (HitCharacter) HitCharacter->MulticastHit(GetActorRotation(), GetActorLocation());
	}
	else
	{
		if (HitCharacter) HitCharacter->SimulateHit(GetActorRotation(), GetActorLocation());
	}
	Destroy();
}

void AProjectileRocket::ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ApplyDamage(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	
	Explode();
}

void AProjectileRocket::Destroyed()
{
	SimulateHit();
	Explode();
	Super::Destroyed();
}
