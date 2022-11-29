// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::FireStart(const FVector& HitTarget)
{
	Super::FireStart(HitTarget);

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (!MyPawn) return;
	AController* InstigatorController = MyPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket && InstigatorController)
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
			if (FireHit.GetActor())
			{
				UGameplayStatics::ApplyDamage(
					FireHit.GetActor(),
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

void AHitScanWeapon::SpawnProjectile(const FVector& HitTarget)
{
	// 什么都不做，不产生子弹
}

