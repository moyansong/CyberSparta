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

	// Destroy 在客户端和服务器都会进行，利用这个来做子弹碰撞的特效和声音
	// 但需要保证子弹不能在复制完成前就Destroy
	virtual void Destroyed() override;

//------------------------------------------Functions----------------------------------------------------------
	UFUNCTION()
	virtual void SimulateHit();

//--------------------------------------------RPC----------------------------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHit();

//------------------------------------------Callbacks----------------------------------------------------------
	// 只在服务器发生碰撞
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
	virtual void ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
//-----------------------------------------Components---------------------------------------------------------
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* TracerComponent;

protected:
//------------------------------------------Parameters----------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "Net")
	bool bShouldSimulateHit = true;

	UPROPERTY(EditDefaultsOnly, Category = "ShellCase")
	TSubclassOf<AShellCase> ShellCaseClass;

	UPROPERTY(EditAnywhere, Category = Damage)
	float Damage = 30.f;
//------------------------------------------Emitters----------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Emitters)
	UParticleSystem* Tracer; // 弹道轨迹特效

	UPROPERTY(EditAnywhere, Category = Emitters)
	UParticleSystem* ImpactParticles;

//------------------------------------------Sounds----------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundCue* ImpactSound;

};
