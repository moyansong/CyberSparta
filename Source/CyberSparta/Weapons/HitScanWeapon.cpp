// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController/MyPlayerController.h"
#include "../Components/LagCompensationComponent.h"

void AHitScanWeapon::FireStart(const FVector& HitTarget)
{
	Super::FireStart(HitTarget);

	if (MyCharacter && MyCharacter->IsLocallyControlled())
	{
		HitScanFire(HitTarget);
	}
}

void AHitScanWeapon::HitScanFire(const FVector& HitTarget)
{
	if (!MyCharacter || !MyController) return;

	const USkeletalMeshSocket* MuzzleSocket = GetMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			AMyCharacter* HitCharacter = Cast<AMyCharacter>(FireHit.GetActor());
			if (HitCharacter)
			{
				MyCharacter->GetLagCompensationComponent()->ServerScoreRequest(
					HitCharacter,
					Start,
					HitTarget,
					MyController->GetServerTime() - (HasAuthority() ? 0.f : MyController->SingleTripTime),// 为什么不直接传Client的时间是因为每个Client的GetWorld()->GetTimeSeconds()时间不一样，与Client加入游戏的时间有关
					this
				);
			}
		}
	}
}

void AHitScanWeapon::SpawnProjectile(const FVector& HitTarget, bool bProjectileUseServerSideRewind, bool bProjectileReplicates)
{
	// 什么都不做，不产生子弹
}

