// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

// 不产生子弹的枪
UCLASS()
class CYBERSPARTA_API AHitScanWeapon : public ARangedWeapon
{
	GENERATED_BODY()

public:

	virtual void FireStart(const FVector& HitTarget) override;

	virtual void SpawnProjectile(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = Emitter)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ImpactSound;
};
