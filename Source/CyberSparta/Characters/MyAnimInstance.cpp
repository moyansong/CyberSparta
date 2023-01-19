// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "MyCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Weapons/Weapon.h"
#include "../CyberSparta.h"
#include "../Types/CombatState.h"
#include "../Components/CombatComponent.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	if (MyCharacter)
	{
		MyMovementComponent = MyCharacter->GetCharacterMovement();
	}
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	if (!MyCharacter) return;
	if (!MyMovementComponent) MyMovementComponent = MyCharacter->GetCharacterMovement();
	if (!MyMovementComponent) return;

	Speed = UKismetMathLibrary::VSizeXY(MyMovementComponent->Velocity);

	bIsCrouched = MyCharacter->bIsCrouched;
	bIsInAir = MyMovementComponent->IsFalling();
	bIsAccelerating = MyMovementComponent->GetCurrentAcceleration().Size() > 0 ? true : false;

	bWeaponEquipped = MyCharacter->IsWeaponEquipped();
	EquippedWeapon = MyCharacter->GetEquippedWeapon();
	EquippedWeaponType = EquippedWeapon ? EquippedWeapon->GetWeaponType() : EWeaponType::EWT_Fist;

	bIsAlive = MyCharacter->IsAlive();
	bIsAiming = MyCharacter->IsAiming();

	HitDirection = MyCharacter->GetHitDirection();
	if (bIsAlive) DeathPoseIndex = FMath::RandRange(0, 3);

	bUseAimOffset = bIsAlive &&
					EquippedWeapon &&
					!MyCharacter->GetDisableGameplay() &&
					MyCharacter->GetCombatState() != ECombatState::ECS_Reloading &&
					MyCharacter->GetCombatState() != ECombatState::ECS_Equipping;

	bUseLeftHandIK = bIsAlive &&
					 EquippedWeapon &&
					 EquippedWeapon->UseLeftHandIK() &&
				     !MyCharacter->GetDisableGameplay() &&
					 MyCharacter->GetCombatState() != ECombatState::ECS_Reloading &&
					 MyCharacter->GetCombatState() != ECombatState::ECS_Equipping;

	bUseRightHandRotation = bIsAlive &&
							bIsAiming &&
							EquippedWeapon &&
							EquippedWeapon->UseRightHandRotation() &&
							!MyCharacter->GetDisableGameplay() &&
							MyCharacter->GetCombatState() != ECombatState::ECS_Reloading &&
							MyCharacter->GetCombatState() != ECombatState::ECS_Equipping;

	FRotator AimRotation = MyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MyCharacter->GetVelocity());
	Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	AimOffset(DeltaTime);
	SetHandTransform(DeltaTime);
}

void UMyAnimInstance::AimOffset(float DeltaTime)
{
	if (MyCharacter && MyCharacter->IsAiming() && bUseAimOffset && bIsAlive)
	{
		FRotator AimRotation = UKismetMathLibrary::NormalizedDeltaRotator(MyCharacter->GetBaseAimRotation(), MyCharacter->GetActorRotation());
		AO_Yaw = AimRotation.Yaw;
		AO_Pitch = AimRotation.Pitch;
	}
	else
	{
		AO_Yaw = 0.f;
		AO_Pitch = 0.f;
	}
}

void UMyAnimInstance::SetHandTransform(float DeltaTime)
{
	if (!MyCharacter || !bWeaponEquipped || !EquippedWeapon->IsRangedWeapon() || !bIsAlive) return;

	USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetMesh();
	USkeletalMeshComponent* CharacterMesh = MyCharacter->GetMesh();

	if (WeaponMesh && CharacterMesh)
	{
		// 让左手放在枪上
		LeftHandTransform = WeaponMesh->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		CharacterMesh->TransformToBoneSpace(
			FName("hand_r"), 
			LeftHandTransform.GetLocation(), 
			FRotator::ZeroRotator, 
			OutPosition, 
			OutRotation
		);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// 调整右手的角度让枪口指向准星指的位置， 在动画蓝图中要用RightHandRotation Transform右手骨骼, 只在本地做，因为准星只有本地可见
		if (MyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = CharacterMesh->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			if (bUseRightHandRotation)
			{
				FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyCharacter->GetHitTarget()));
				RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 25.f);
			}
			else
			{
				RightHandRotation = RightHandTransform.Rotator();
			}
		}

		// Debug枪口指向和准星指向的射线
		/*FTransform MuzzleTipTransform = EquippedWeapon->GetMesh()->GetSocketTransform(FName("Muzzle"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX = FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MyCharacter->GetHitTarget(), FColor::Orange);*/
	}
}


