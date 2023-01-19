// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "ShellCase.h"
#include "../Characters/MyCharacter.h"
#include "../CyberSparta.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	InitialLifeSpan = 5.f;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetBoxExtent(FVector(10.f, 5.f, 5.f));
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bReplicates)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
	else if (HasAuthority())
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionComponent,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	//AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetOwner());
	//if (MyCharacter)
	//{
	//	bShouldSimulateHit = MyCharacter->ShouldMulticastEffect();
	//}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	/*if (bShouldSimulateHit)
	{
		SimulateHit();
	}*/
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		Destroy();
		return;
	}

	AMyCharacter* HitCharacter = Cast<AMyCharacter>(OtherActor);
	if (bReplicates && HasAuthority())
	{
		MulticastHit();
		if (HitCharacter) HitCharacter->MulticastHit(GetActorLocation());
	}
	else
	{
		SimulateHit(); 
		if (HitCharacter) HitCharacter->SimulateHit(GetActorLocation());
	}

	if (HitCharacter && HitCharacter->IsAlive())
	{
		ApplyDamage(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}

	Destroy();
}

void AProjectile::SimulateHit()
{
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), 
			ImpactEffect, 
			GetActorTransform()
		);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			ImpactSound, 
			GetActorLocation()
		);
	}
}

void AProjectile::MulticastHit_Implementation()
{
	if (bShouldSimulateHit)
	{
		SimulateHit();
	}
}

void AProjectile::ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
}

void AProjectile::Explode()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				DamageInnerRadius,
				DamageOuterRadius,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController,
				ECC_RadialDamage
			);
		}
	}
}
