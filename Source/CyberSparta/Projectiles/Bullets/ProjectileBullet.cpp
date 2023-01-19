// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Weapons/Weapon.h"
#include "../../Game/PlayerControllers/MyPlayerController.h"
#include "../../Components/LagCompensationComponent.h"
#include "../../GameFramework/HeadShotDamageType.h"
#include "../../GameFramework/TrunkShotDamageType.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*FPredictProjectilePathParams PredictParams;
	PredictParams.bTraceWithChannel = true;
	PredictParams.bTraceWithCollision = true;
	PredictParams.DrawDebugTime = 5.f;
	PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PredictParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PredictParams.MaxSimTime = 4.f;
	PredictParams.ProjectileRadius = 5.f;
	PredictParams.SimFrequency = 100.f;
	PredictParams.StartLocation = GetActorLocation();
	PredictParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PredictParams.ActorsToIgnore.Add(this);
	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);*/
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); 
}

void AProjectileBullet::ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ApplyDamage(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetOwner());
	if (MyCharacter)
	{
		AMyPlayerController* MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			if (bUseServerSideRewind)
			{
				AMyCharacter* HitCharacter = Cast<AMyCharacter>(OtherActor);
				if (HitCharacter && MyCharacter->GetLagCompensationComponent() && MyCharacter->IsLocallyControlled())
				{
					MyCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(
						HitCharacter,
						TraceStart,
						InitialVelocity,
						MyController->GetServerTime() - MyController->SingleTripTime
					);
				}
			}
			else
			{
				if (HasAuthority() && MyCharacter->GetEquippedWeapon() && !MyCharacter->GetEquippedWeapon()->UseServerSideRewind())
				{
					float DamageToCause = Damage;
					TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

					FName HitBone = Hit.BoneName;
					if (HitBone == FName("Head"))
					{
						DamageTypeClass = UHeadShotDamageType::StaticClass();
						DamageToCause = HeadShotDamage;
					}
					else if (HitBone == FName("spine_01") || HitBone == FName("spine_02") || HitBone == FName("spine_03"))
					{
						DamageTypeClass = UTrunkShotDamageType::StaticClass();
					}

					UGameplayStatics::ApplyDamage(
						OtherActor,
						DamageToCause,
						MyController,
						MyCharacter->GetEquippedWeapon(),
						DamageTypeClass
					); 
				}
			}
		}
	}

}
