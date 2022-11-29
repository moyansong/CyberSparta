// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Camera/CameraComponent.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Projectiles/Projectile.h"
#include "../Projectiles/ShellCase.h"

ARangedWeapon::ARangedWeapon()
{
	bIsRangedWeapon = true;

	DefaultFOV = 90.f;
	CurrFOV = 90.f;
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

void ARangedWeapon::FireStart(const FVector& HitTarget)
{
	Super::FireStart(HitTarget);

	// 只在服务器生成子弹，复制到客户端
	if (HasAuthority())
	{
		// 拓展：HitTarget加上后坐力
		SpawnProjectile(HitTarget);
	}
}

void ARangedWeapon::SimulateFire()
{
	Super::SimulateFire();
	
	// 借由广播在每个客户端生成一个，不是复制
	SpawnShellCase();
}

void ARangedWeapon::SpawnProjectile(const FVector& HitTarget)
{
	if (!HasAuthority() || !ProjectileClass) return;

	USkeletalMeshComponent* Mesh = GetMesh();
	UWorld* World = GetWorld();
	if (Mesh && World && GetOwner())
	{
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		const USkeletalMeshSocket* MuzzleSocket = Mesh->GetSocketByName(FName("Muzzle"));
		if (MuzzleSocket && InstigatorPawn)
		{
			FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(Mesh);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			FVector ProjectileDirection = HitTarget - MuzzleTransform.GetLocation();
			FRotator ProjectileRotation = ProjectileDirection.Rotation();

			AProjectile* Projectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				MuzzleTransform.GetLocation(),
				ProjectileRotation,
				SpawnParams
			);
		}
	}
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

	if (!GetOwner()) return;

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter)
	{
		DefaultFOV = MyCharacter->GetFollowCamera()->FieldOfView;
	}
}
