// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileGrenade.h"
#include "ProjectileSmokeGrenade.generated.h"

// 烟雾弹，没有伤害
UCLASS()
class CYBERSPARTA_API AProjectileSmokeGrenade : public AProjectileGrenade
{
	GENERATED_BODY()
	
public:
	virtual void Destroyed() override;
};
