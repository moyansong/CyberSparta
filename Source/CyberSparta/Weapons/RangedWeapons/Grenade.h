// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Grenade.generated.h"

UCLASS()
class CYBERSPARTA_API AGrenade : public ARangedWeapon
{
	GENERATED_BODY()

public:
	AGrenade();

	virtual void FireStart(const FVector& HitTarget) override;
	virtual void FireStop() override;

	// 在动画通知里调用
	void SpawnGrenade();

	// 扔手雷的蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* ThrowMontage;

};
