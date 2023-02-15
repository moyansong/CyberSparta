// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PooledObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPooledObjectDespawnSignature, APooledObject*, PooledObject);

UCLASS()
class OBJECTPOOL_API APooledObject : public AActor
{
	GENERATED_BODY()
	
public:	
	APooledObject();
	virtual void Tick(float DeltaTime) override;

	FOnPooledObjectDespawnSignature OnPooledObjectDespawn;

//------------------------------------------Set && Get---------------------------------------------------------
	virtual void SetActive(bool IsActive);
	FORCEINLINE bool IsActive() const { return bIsActive; }

	void SetObjectLifeSpan(float InLifeSpan) { ObjectLifeSpan = InLifeSpan; }

//------------------------------------------Functions---------------------------------------------------------

protected:
	virtual void BeginPlay() override;

//------------------------------------------Parameters---------------------------------------------------------

	bool bIsActive = false;

	float ObjectLifeSpan = 0.f;
};
