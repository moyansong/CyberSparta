// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;

UCLASS()
class CYBERSPARTA_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	
private:
	// 这个组件是为了处理ProjectileMovementComponent的Hit事件，ProjectileMovementComponent触发Hit会停止移动，
	// 我们在CollisionComponent中忽略了与自己的Hit事件
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;

public:
	AProjectileRocket();
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void Destroyed() override;
};
