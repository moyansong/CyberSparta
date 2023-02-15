// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPoolComponent.h"
#include "PooledObject.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UObjectPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UObjectPoolComponent::AddPooledObject(APooledObject* PooledObject)
{
	ObjectPool.Enqueue(PooledObject);
}

APooledObject* UObjectPoolComponent::SpawnPooledObject()
{
	if (UWorld* World = GetWorld())
	{
		if (PooledObjectClass)
		{
			APooledObject* PooledObject = World->SpawnActor<APooledObject>(
				PooledObjectClass,
				FVector::ZeroVector,
				FRotator::ZeroRotator
			);

			if (PooledObject)
			{
				AddPooledObject(PooledObject);
				PooledObject->SetActive(false);
				PooledObject->OnPooledObjectDespawn.AddDynamic(this, &UObjectPoolComponent::OnPooledObjectDespawn);
				
				return PooledObject;
			}
		}
	}
	return nullptr;
}

APooledObject* UObjectPoolComponent::GetPooledObject()
{
	APooledObject* PooledObject = nullptr;
	if (ObjectPool.Dequeue(PooledObject))
	{
		return PooledObject;
	}
	return SpawnPooledObject();
}

void UObjectPoolComponent::OnPooledObjectDespawn(APooledObject* PooledObject)
{
	AddPooledObject(PooledObject);
}