// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParctileSystem;
class UParctileSystemComponent;
class USoundCue;
class AShellCase;

UCLASS()
class CYBERSPARTA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
public:	
	friend class ARangedWeapon;

	AProjectile();
	virtual void Tick(float DeltaTime) override;

	// Destroy在客户端和服务器都会进行，利用这个来做子弹碰撞的特效和声音
	// 但需要保证子弹不能在复制完成前就Destroy
	virtual void Destroyed() override;

//------------------------------------------Functions----------------------------------------------------------
	FORCEINLINE UBoxComponent* GetCollisionComponent() { return CollisionComponent; }
	FORCEINLINE UStaticMeshComponent* GetMesh() { return MeshComponent; }

	// OnHit会处理与网络有关的事，比如广播特效，在Server上产生伤害
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	virtual void ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();
//--------------------------------------------RPC----------------------------------------------------------
	UFUNCTION()
	virtual void SimulateHit();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();
private:
//-----------------------------------------Components---------------------------------------------------------
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionComponent;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* TracerComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent; // 子类里创建
//------------------------------------------Parameters----------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = Net)
	bool bShouldSimulateHit = true;

	UPROPERTY(EditDefaultsOnly, Category = ShellCase)
	TSubclassOf<AShellCase> ShellCaseClass;

	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage = 30.f;

	// 范围伤害用的
	UPROPERTY(EditAnywhere, Category = Damage)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere, Category = Damage)
	float DamageOuterRadius = 500.f;
//------------------------------------------Emitters----------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Emitter)
	UParticleSystem* Tracer; // 弹道轨迹特效

	UPROPERTY(EditAnywhere, Category = Emitter)
	UParticleSystem* ImpactParticles;

//------------------------------------------Sounds----------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ImpactSound;

};
