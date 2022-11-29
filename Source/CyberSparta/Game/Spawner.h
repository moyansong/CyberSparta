// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

// 产生一些道具，道具存在一定时间后会销毁，销毁或被人捡走后过一段时间会刷新
UCLASS()
class CYBERSPARTA_API ASpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawner();
	virtual void Tick(float DeltaTime) override;

//--------------------------------------------Functions-------------------------------------------------------------
	void SpawnProduct();

	UFUNCTION()
	void StartSpawnTimer();

	UFUNCTION()
	void StartDestroyTimer();

protected:
	virtual void BeginPlay() override;

	void SpawnTimerFinished();
	void DestroyTimerFinished();

//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AActor>> ProductClasses;

	UPROPERTY()
	AActor* Product;

private:
	FTimerHandle SpawnTimer;
	FTimerHandle DestroyTimer;

	// Spawn的时间是这两值中间的随机值
	UPROPERTY(EditAnywhere, Category = Time)
	float MinSpawnTime = 20.f;
	UPROPERTY(EditAnywhere, Category = Time)
	float MaxSpawnTime = 25.f;

	// Product销毁的时间是这两值中间的随机值
	UPROPERTY(EditAnywhere, Category = Time)
	float MinDestroyTime = 10.f;
	UPROPERTY(EditAnywhere, Category = Time)
	float MaxDestroyTime = 15.f;

	bool bIsSpawning = false;
};
