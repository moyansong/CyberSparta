// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../../CyberSparta.h"

AProjectileGrenade::AProjectileGrenade()
{
	InitialLifeSpan = 5.f;
	Damage = 70.f;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 1500.f;
	ProjectileMovementComponent->ProjectileGravityScale = 1.f;
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		12.f,
		12,
		FColor::Red,
		false,
		10.f
	);*/
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
	GetCollisionComponent()->OnComponentHit.RemoveAll(this);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	SimulateHit();
	Explode();
	Super::Destroyed();
}