// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "MyDamageType.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API UMyDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = Parameter)
	float HeadDamage;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float ChestDamage;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float BodyDamage;
};
