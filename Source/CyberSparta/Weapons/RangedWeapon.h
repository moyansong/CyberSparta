// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "RangedWeapon.generated.h"

class AProjectile;
class AShellCase;

/**
 * 
 */
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
	
	virtual void FireStart(const FVector& HitTarget) override;

	virtual void SimulateFire() override;

	// bProjectileUseServerSideRewind = true表示子弹需要向Server报告击中结果，由Server检查
	virtual void SpawnProjectile(const FVector& HitTarget, bool bProjectileUseServerSideRewind, bool bProjectileReplicates = true);

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
	float TargetFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float TargetInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float DefaultFOV;

	UPROPERTY(VisibleAnywhere, Category = Parameter)
	float CurrFOV;

	UPROPERTY(VisibleAnywhere, Category = Parameter)
	bool bIsTargeting = false;

	UPROPERTY(VisibleAnywhere, Category = Parameter)
	bool bCanTarget = true;
private:
};
