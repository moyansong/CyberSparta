// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "../../CyberSparta.h"
#include "../../Types/CombatState.h"
#include "../../Characters/MyCharacter.h"
#include "../../Game/PlayerControllers/MyPlayerController.h"
#include "../../Components/LagCompensationComponent.h"

AShotgun::AShotgun()
{
	Ammo = 6;
	MaxAmmo = 6;
	Damage = 10.f;
	FireDelay = 1.f;
	ReloadDuration = 2.f;
	HeadShotDamage = 20.f;
	bCanTarget = false;
	bUseScatter = true;
	bUseLeftHandIK = true;
	bCanAutomaticFire = false;
	bUseServerSideRewind = true;
	bUseRightHandRotation = true;
	WeaponType = EWeaponType::EWT_Shotgun;
}

void AShotgun::FireStart(const FVector& HitTarget)
{
	AWeapon::FireStart(HitTarget);

	const USkeletalMeshSocket* MuzzleSocket = GetMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket && MyCharacter)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TArray<AMyCharacter*> HitCharacters;
		TArray<FVector_NetQuantize> HitLocations;
		for (uint32 i = 0; i < PelletNum; ++i)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			if (FireHit.bBlockingHit)
			{
				AMyCharacter* HitCharacter = Cast<AMyCharacter>(FireHit.GetActor());
				if (HitCharacter)
				{
					HitCharacters.AddUnique(HitCharacter);
					HitLocations.Add(FireHit.ImpactPoint);
				}
				SimulateHit(FireHit);
			}
		}

		MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			if (bUseServerSideRewind)
			{
				if (MyCharacter->IsLocallyControlled())
				{
					MyCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
						HitCharacters,
						Start,
						HitLocations,
						MyController->GetServerTime() - MyController->SingleTripTime,
						this
					);
				}
			}
			else
			{
				if (HasAuthority())
				{

				}
			}
		}
	}
}

bool AShotgun::UseRightHandRotation()
{
	return MyCharacter && MyCharacter->GetCombatState() == ECombatState::ECS_Idle;
}
