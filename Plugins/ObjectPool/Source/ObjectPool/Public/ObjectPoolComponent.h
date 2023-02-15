// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"

class APooledObject;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OBJECTPOOL_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UObjectPoolComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//------------------------------------------Set && Get---------------------------------------------------------

	virtual APooledObject* GetPooledObject();

	template<class T>
	T* GetPooledObject()
	{
		return Cast<T>(GetPooledObject());
	}

//------------------------------------------Functions---------------------------------------------------------
	virtual void AddPooledObject(APooledObject* PooledObject);

	virtual APooledObject* SpawnPooledObject();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPooledObjectDespawn(APooledObject* PooledObject);

//------------------------------------------Paramerters---------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = ObjectPool)
	TSubclassOf<APooledObject> PooledObjectClass;

	UPROPERTY(EditAnywhere, Category = ObjectPool)
	float PooledObjectLifeSpan = 0.f;

	TQueue<APooledObject*> ObjectPool;

private:

};
