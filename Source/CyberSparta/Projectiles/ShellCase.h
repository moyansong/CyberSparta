// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellCase.generated.h"

class UStaticMeshComponent;

UCLASS()
class CYBERSPARTA_API AShellCase : public AActor
{
	GENERATED_BODY()
	
public:	
	AShellCase();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComponent;

	// 弹壳弹出时的冲量最小值
	UPROPERTY(EditAnywhere, Category = Parameters)
	float MinShellEjectionImpluse = 10.f;

	// 弹壳弹出时的冲量最大值
	UPROPERTY(EditAnywhere, Category = Parameters)
	float MaxShellEjectionImpluse = 15.f;
};
