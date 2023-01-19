// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../Weapons/WeaponTypes.h"
#include "CharacterTypes.h"
#include "MyAnimInstance.generated.h"

class AMyCharacter;
class AWeapon;
class UCharacterMovementComponent;

UCLASS()
class CYBERSPARTA_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	friend class AMyCharacter;

	// 类似于BeginPlay
	virtual void NativeInitializeAnimation() override;

	// 类似于Tick，总是运行，即使没在游戏中，所以空指针会导致编辑器崩溃
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	void AimOffset(float DeltaTime);

	void SetHandTransform(float DeltaTime);

private:
//------------------------------------------Parameters----------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = Paramters, meta = (AllowPrivateAccess = "true"))
	AMyCharacter* MyCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCharacterMovementComponent* MyMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = Paramters, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Paramters, meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	UPROPERTY(BlueprintReadOnly, Category = Paramters, meta = (AllowPrivateAccess = "true"))
	EHitDirection HitDirection = EHitDirection::EHD_Forward;

	UPROPERTY(BlueprintReadOnly, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	int32 DeathPoseIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Direction;

	//身体的倾斜度
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;	

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	// 拿枪时左手的位置,确保你的武器有个LeftHandSocket
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform; 

	// 旋转右手使得枪口指向准星
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseLeftHandIK; 

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseRightHandRotation; 

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAlive = true;

//------------------------------------------Animations----------------------------------------------------------
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (AllowPrivateAccess = "true"))
	
};
