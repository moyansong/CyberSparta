// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Buff.h"
#include "HealBuff.generated.h"


UCLASS()
class CYBERSPARTA_API AHealBuff : public ABuff
{
	GENERATED_BODY()

public:
	AHealBuff();
	virtual void TickEffective(float DeltaTime) override;

private:
//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Parameter)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float HealingTime = 10.f;

	float HealingRate = 0.f;
};
