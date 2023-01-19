// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "../WeaponTypes.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Game/PlayerControllers/MyPlayerController.h"
#include "../../Components/LagCompensationComponent.h"

void AHitScanWeapon::FireStart(const FVector& HitTarget)
{
	AWeapon::FireStart(HitTarget);

	const USkeletalMeshSocket* MuzzleSocket = GetMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket && MyCharacter)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		SimulateHit(FireHit);

		AMyCharacter* HitCharacter = Cast<AMyCharacter>(FireHit.GetActor());
		if (bUseServerSideRewind)
		{
			if (HitCharacter && MyCharacter->IsLocallyControlled() && MyController)
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
		else
		{
			if (HasAuthority())
			{
				// 直接施加伤害
			}
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * SphereDistance;
	FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLocation = SphereCenter + RandVector;
	FVector ToEndLocation = EndLocation - TraceStart;

	//DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Blue, true);
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::Cyan, true);

	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamEffect)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamEffect,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AHitScanWeapon::SimulateHit(const FHitResult& HitResult)
{
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactEffect,
			HitResult.ImpactPoint,
			HitResult.ImpactNormal.Rotation()
		);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			HitResult.ImpactPoint
		);
	}
}
