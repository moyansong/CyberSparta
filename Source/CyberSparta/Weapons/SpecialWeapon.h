// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "SpecialWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API ASpecialWeapon : public AWeapon
{
	GENERATED_BODY()
public:
//---------------------------------------------Functions-------------------------------------------------------------
	ASpecialWeapon();

	virtual void Drop() override;
	virtual void Throw(FVector ThrowDirection, float Force) override;

	virtual void OnEquipped() override;
	virtual void OnDropped() override;
	virtual void OnIdled() override;

	FORCEINLINE UStaticMeshComponent* GetStaticMesh() const { return StaticMeshComponent; }
//---------------------------------------------Parameters-------------------------------------------------------------
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;
};
