// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawner.h"
#include "../CyberSparta.h"

ASpawner::ASpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ASpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Product && Product->GetOwner() != this)
	{
		StartSpawnTimer();
	}
}

void ASpawner::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimer();
}

void ASpawner::StartSpawnTimer()
{
	if (bIsSpawning || !HasAuthority()) return;
	const float SpawnTime = FMath::FRandRange(MinSpawnTime, MaxSpawnTime);
	GetWorldTimerManager().SetTimer(
		SpawnTimer,
		this,
		&ASpawner::SpawnTimerFinished,
		SpawnTime
	); 
	bIsSpawning = true;
}

void ASpawner::StartDestroyTimer()
{
	if (!Product || !HasAuthority()) return;
	const float DestroyTime = FMath::FRandRange(MinDestroyTime, MaxDestroyTime);
	GetWorldTimerManager().SetTimer(
		SpawnTimer,
		this,
		&ASpawner::DestroyTimerFinished,
		DestroyTime
	);
	bIsSpawning = false;
}

void ASpawner::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnProduct();
	}
}

void ASpawner::DestroyTimerFinished()
{
	if (Product && HasAuthority())
	{
		if (Product->GetOwner() == this)
		{
			Product->Destroy();
		}
		if (!bIsSpawning)
		{
			StartSpawnTimer();
		}
	}
}

void ASpawner::SpawnProduct()
{
	int32 ProductClassesNum = ProductClasses.Num();
	if (ProductClassesNum > 0)
	{
		// Fix me : 当人物就站在Spawner上，此时产生东西，即使与人物重叠也不会触发Overlap事件，要人物走出去再进来才行
		FActorSpawnParameters SpawnParams; 
		//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = this;
		int32 ProductClassIndex = FMath::RandRange(0, ProductClassesNum - 1);
		Product = GetWorld()->SpawnActor<AActor>(
			ProductClasses[ProductClassIndex],
			GetActorTransform(),
			SpawnParams
		);
		
		if (Product && HasAuthority())
		{
			StartDestroyTimer();
		}
	}
}


