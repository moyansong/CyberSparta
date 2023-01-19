// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Projectiles/Projectile.h"

AGrenade::AGrenade()
{
	Ammo = 1;
	MaxAmmo = 1;
	AmmoCostPerFire = 1;
	FireDelay = 1.5f;
	bCanDrop = false;
	bReplicates = true;
	bCanReload = false;
	bCanTarget = false;
	bUseLeftHandIK = false;
	bCanAutomaticFire = false;
	bUseServerSideRewind = false;
	bUseRightHandRotation = false;
	WeaponType = EWeaponType::EWT_Grenade;
}

void AGrenade::FireStart(const FVector& HitTarget)
{
	SpawnShellCase();
}

void AGrenade::FireStop()
{
	if (!CanFire() || !MyCharacter) return;

	SpendRound();
	MyCharacter->PlayAnimMontage(ThrowMontage);
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			ThrowTimer,
			this,
			&AGrenade::SpawnGrenade,
			0.35f,
			false
		);
	}
}

void AGrenade::SpawnGrenade()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	UWorld* World = GetWorld();
	if (Mesh && World && GetOwner() && ProjectileClass && MyCharacter)
	{
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		const USkeletalMeshSocket* MuzzleSocket = Mesh->GetSocketByName(FName("Muzzle"));
		if (MuzzleSocket && InstigatorPawn)
		{
			FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(Mesh);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			FVector NoUse;
			FRotator ProjectileRotation;
			MyCharacter->GetActorEyesViewPoint(NoUse, ProjectileRotation);
			ProjectileRotation.Pitch += 45.f;

			AProjectile* Projectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				MuzzleTransform.GetLocation(),
				ProjectileRotation,
				SpawnParams
			);
			if (Projectile)
			{
				Projectile->SetUseServerSideRewind(false);
				Projectile->SetReplicates(true);
			}
		}
	}
}
