// Fill out your copyright notice in the Description page of Project Settings.


#include "HealBuff.h"
#include "../Characters/MyCharacter.h"
#include "../Components/AttributeComponent.h"

AHealBuff::AHealBuff()
{
	HealingRate = HealAmount / HealingTime;
}

void AHealBuff::TickEffective(float DeltaTime)
{
	Super::TickEffective(DeltaTime);

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (!MyCharacter || !MyCharacter->IsAlive()) return;
	
	const float HealThisFrame = HealingRate * DeltaTime;
	MyCharacter->GetAttributeComponent()->IncreaseHealth(HealThisFrame);
}
