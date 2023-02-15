// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon.h"
#include "RangedWeapon.generated.h"

class AProjectile;
class AShellCase;
class USoundCue;
class UObjectPoolComponent;

UCLASS()
class CYBERSPARTA_API ARangedWeapon : public AWeapon
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

public:
//--------------------------------------------Functions---------------------------------------------------------
	ARangedWeapon();
	virtual void Tick(float DeltaTime) override;
	
	virtual bool CanFire() override;

	virtual void FireStart(const FVector& HitTarget) override;

	virtual void SimulateFire() override;

	// bProjectileUseServerSideRewind = true表示子弹需要向Server报告击中结果，由Server检查
	virtual AProjectile* SpawnProjectile(const FVector& HitTarget, bool bProjectileUseServerSideRewind, bool bProjectileReplicates = true);

	// 产生弹壳，需要确保武器SKeletalMesh里有AmmoEject套接字
	virtual void SpawnShellCase();

	UFUNCTION()
	void TargetStart();
	UFUNCTION()
	void TargetStop();

	void InterpTargetFOV(float DeltaTime);

	FORCEINLINE bool IsTargeting() const { return bIsTargeting; }
	FORCEINLINE bool CanTarget() const { return bCanTarget; }

	virtual	void Equip() override;

	AProjectile* GetDefaultProjectile();

protected:
//--------------------------------------------Parameters---------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Projectile)
	TSubclassOf<AProjectile> ProjectileClass;	

	UPROPERTY(EditAnywhere, Category = Projectile)
	TSubclassOf<AShellCase> ShellCaseClass;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bUseProjectilePool = false;

	UPROPERTY(EditAnywhere, Category = Target)
	float TargetFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Target)
	float TargetInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Target)
	float DefaultFOV;

	UPROPERTY(VisibleAnywhere, Category = Target)
	float CurrFOV;

	UPROPERTY(VisibleAnywhere, Category = Target)
	bool bIsTargeting = false;

	UPROPERTY(VisibleAnywhere, Category = Target)
	bool bCanTarget = true;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ProjectileDrySound;

private:
	// 存储子弹的对象池
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UObjectPoolComponent* ProjectilePoolComponent;
};
