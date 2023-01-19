// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlastingZone.generated.h"

class USphereComponent;

// 爆破模式用的，放炸弹的区域
UCLASS()
class CYBERSPARTA_API ABlastingZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlastingZone();
	virtual void Tick(float DeltaTime) override;

//-----------------------------------------------Functions-----------------------------------------------------

protected:
	virtual void BeginPlay() override;

	// 碰撞球的体积很大，设置一些属性，比如红队的人可以安炸弹，蓝队的可以拆炸弹
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
//-----------------------------------------------Parameters-----------------------------------------------------
private:
	UPROPERTY(EditAnywhere, Category = Component)
	USphereComponent* SphereComponent;
};
