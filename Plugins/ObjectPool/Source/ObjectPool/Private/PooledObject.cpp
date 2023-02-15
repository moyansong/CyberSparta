// Fill out your copyright notice in the Description page of Project Settings.


#include "PooledObject.h"

APooledObject::APooledObject()
{
	PrimaryActorTick.bCanEverTick = true;

}

void APooledObject::BeginPlay()
{
	Super::BeginPlay();
	
}

void APooledObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APooledObject::SetActive(bool IsActive)
{
	bIsActive = IsActive;
	SetActorHiddenInGame(!bIsActive);
	if (!bIsActive)
	{
		OnPooledObjectDespawn.Broadcast(this);
	}
}

