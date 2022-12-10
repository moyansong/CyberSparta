// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpecialWeapon.h"
#include "BlastingBomb.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API ABlastingBomb : public ASpecialWeapon
{
	GENERATED_BODY()

//---------------------------------------------Functions-------------------------------------------------------------
public:
	ABlastingBomb();
	virtual void Tick(float DeltaTime) override;

	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	// 红队用互动键捡起，开火键安炸弹，蓝队用互动拆炸弹？
	virtual void InteractStart(AActor* InteractActor) override;
	virtual void InteractStop(AActor* InteractActor) override;
//---------------------------------------------Parameters-------------------------------------------------------------
private:

};
