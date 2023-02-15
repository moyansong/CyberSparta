// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "PooledObject.h"
#include "ObjectPoolComponent.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Projectiles/Projectile.h"
#include "../../Projectiles/ShellCase.h"

ARangedWeapon::ARangedWeapon()
{
	bIsRangedWeapon = true;

	DefaultFOV = 90.f;
	CurrFOV = 90.f;

	ProjectilePoolComponent = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("ProjectilePoolComponent"));
	ProjectilePoolComponent->SetIsReplicated(true);
}

void ARangedWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void ARangedWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	InterpTargetFOV(DeltaTime);
}

bool ARangedWeapon::CanFire()
{
	if (Ammo < AmmoCostPerFire)
	{
		if (ProjectileDrySound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				ProjectileDrySound,
				GetActorLocation()
			);
		}
		return false;
	}
	return Super::CanFire();
}

void ARangedWeapon::FireStart(const FVector& HitTarget)
{
	Super::FireStart(HitTarget);
	if (!MyCharacter) return;

	if (bUseServerSideRewind) 
	{
		// 在每个端都生成子弹，且不复制，但只有本地端需要向Server发RPC确认Hit结果
		SpawnProjectile(HitTarget, MyCharacter->IsLocallyControlled(), false);
	}
	else if (HasAuthority())
	{
		// 只在Server产生子弹，也只在Server碰撞，子弹需要复制
		SpawnProjectile(HitTarget, false, true);
	}
}

void ARangedWeapon::SimulateFire()
{
	Super::SimulateFire();
	
	SpawnShellCase();
}

AProjectile* ARangedWeapon::SpawnProjectile(const FVector& HitTarget, bool bProjectileUseServerSideRewind, bool bProjectileReplicates)
{
	USkeletalMeshComponent* Mesh = GetMesh();
	UWorld* World = GetWorld();
	if (Mesh && World && GetOwner() && ProjectileClass)
	{
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		const USkeletalMeshSocket* MuzzleSocket = Mesh->GetSocketByName(FName("Muzzle"));
		if (MuzzleSocket && InstigatorPawn)
		{
			FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(Mesh);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			FVector ProjectileLocation = MuzzleTransform.GetLocation();
			FVector ProjectileDirection = HitTarget - ProjectileLocation;
			FRotator ProjectileRotation = ProjectileDirection.Rotation();

			AProjectile* Projectile = nullptr;

			if (bUseProjectilePool)
			{
				Projectile = ProjectilePoolComponent->GetPooledObject<AProjectile>();
				if (Projectile)
				{
					Projectile->SetOwner(GetOwner());
					Projectile->SetInstigator(InstigatorPawn);
					Projectile->SetActorLocation(ProjectileLocation);
					Projectile->SetActorRotation(ProjectileRotation);
					Projectile->bUseProjectilePool = bUseProjectilePool;
					Projectile->SetActive(true);
				}
			}
			else
			{
				Projectile = World->SpawnActor<AProjectile>(
					ProjectileClass,
					ProjectileLocation,
					ProjectileRotation,
					SpawnParams
				);
			}

			if (Projectile)
			{
				Projectile->bUseServerSideRewind = bProjectileUseServerSideRewind;
				Projectile->bReplicates = bProjectileReplicates;
				Projectile->TraceStart = MuzzleTransform.GetLocation();
				Projectile->InitialVelocity = Projectile->GetActorForwardVector() * Projectile->InitialSpeed;
				return Projectile;
			}
		}
	}
	return nullptr;
}

void ARangedWeapon::SpawnShellCase()
{
	if (!ShellCaseClass) return;
	
	USkeletalMeshComponent* Mesh = GetMesh();
	UWorld* World = GetWorld();
	if (Mesh && World && GetOwner())
	{
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		const USkeletalMeshSocket* AmmoEjectSocket = Mesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket && InstigatorPawn)
		{
			FTransform AmmoEjectTransform = AmmoEjectSocket->GetSocketTransform(Mesh);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			AShellCase* ShellCase = World->SpawnActor<AShellCase>(
				ShellCaseClass,
				AmmoEjectTransform.GetLocation(),
				AmmoEjectTransform.GetRotation().Rotator(),
				SpawnParams
			);
		}
	}
}

void ARangedWeapon::TargetStart()
{
	bIsTargeting = true;
}

void ARangedWeapon::TargetStop()
{
	bIsTargeting = false;
}

void ARangedWeapon::InterpTargetFOV(float DeltaTime)
{
	if (!GetOwner()) return;

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (!MyCharacter || !MyCharacter->GetFollowCamera()) return;

	if (bIsTargeting)
	{
		CurrFOV = FMath::FInterpTo(CurrFOV, TargetFOV, DeltaTime, TargetInterpSpeed);
	}
	else
	{
		CurrFOV = FMath::FInterpTo(CurrFOV, DefaultFOV, DeltaTime, TargetInterpSpeed);
	}
	
	MyCharacter->GetFollowCamera()->SetFieldOfView(CurrFOV);
}

void ARangedWeapon::Equip()
{
	Super::Equip();

	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetOwner());
	if (MyCharacter)
	{
		DefaultFOV = MyCharacter->GetFollowCamera()->FieldOfView;
	}
}

AProjectile* ARangedWeapon::GetDefaultProjectile()
{
	return ProjectileClass ? Cast<AProjectile>(ProjectileClass->GetDefaultObject()) : nullptr;
}
