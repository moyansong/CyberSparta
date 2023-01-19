// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeapon.h"
#include "Pistol.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API APistol : public ARangedWeapon
{
	GENERATED_BODY()
	
public:
	APistol();
	
	// Fix me:按鼠标右键用手枪敲人
};
