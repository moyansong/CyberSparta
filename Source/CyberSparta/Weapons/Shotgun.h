// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

UCLASS()
class CYBERSPARTA_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:
	AShotgun();

	virtual void FireStart(const FVector& HitTarget) override;

	virtual bool UseRightHandRotation() override;
private:
	// 一次开火打出的弹丸数
	UPROPERTY(EditAnywhere, Category = Ammo)
	uint32 PelletNum = 10;
};
