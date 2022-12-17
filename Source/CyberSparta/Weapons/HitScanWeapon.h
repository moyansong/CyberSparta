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

//---------------------------------------------Functions-------------------------------------------------------------
public:
	virtual void FireStart(const FVector& HitTarget) override;

	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }

protected:
	// 返回一个点，其与TraceStart的连线会穿过SphereDistance和SphereRadius定义的球内随机某个位置
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	void SimulateHit(const FHitResult& HitResult);
//---------------------------------------------Parameters-------------------------------------------------------------
	// 类似Shougun的武器用的，其会在Distance后的球上随机选取几个点来生成多条射线
	UPROPERTY(EditAnywhere, Category = Parameter)
	float SphereDistance = 600.f;

	// 类似Shougun的武器用的，其会在Distance后的球上随机选取几个点来生成多条射线,这是球的半径
	UPROPERTY(EditAnywhere, Category = Parameter)
	float SphereRadius = 100.f;

	// 散弹枪用，弹道随机
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bUseScatter = false;

	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float HeadShotDamage = 100.f;

	UPROPERTY(EditAnywhere, Category = Effect)
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ImpactSound;

	// 展示弹道用的
	UPROPERTY(EditAnywhere, Category = Effect)
	UParticleSystem* BeamEffect;
};
