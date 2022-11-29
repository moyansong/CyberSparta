// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractInterface.h"
#include "../Weapons/WeaponTypes.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UCombatComponent;
class UTexture2D;
class AMyCharacter;
class SoundCue;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),// 枪刚初始化在世界里
	EWS_Equipped UMETA(DisplayName = "Equipped"),// 正在使用的武器
	EWS_Dropped UMETA(DisplayName = "Dropped"),// 被人丢了
	EWS_Idle UMETA(DisplayName = "Idle"),// 在人身上但是现在不在用
	EWS_MAX UMETA(DisplayName = "DefaultMax") // 这个是为了告诉别人这个枚举有几个(枚举是整数)
};

UCLASS()
class CYBERSPARTA_API AWeapon : public AActor, public IInteractInterface
{
	GENERATED_BODY()
public:	
	friend class UCombatComponent;

	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//--------------------------------------------Textures-------------------------------------------------------------
	// 准星
	UPROPERTY(EditAnywhere, Category = Texture)
	UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = Texture)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Texture)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = Texture)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Texture)
	UTexture2D* CrosshairsBottom;

	// 该值越大，准星最大扩张就越大
	float CorsshiarsSpreadScale = 1.f;
//--------------------------------------------Animation-------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Animation)
	UAnimationAsset* FireAnimation; // 武器自己开火的动画

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* FireMontage; // 角色开火的蒙太奇

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Animation)
	UAnimationAsset* ReloadAnimation; // 武器自己换弹的动画

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* ReloadMontage; // 角色换弹的蒙太奇
//----------------------------------------------Sounds-------------------------------------------------------------
	
//---------------------------------------------Functions-------------------------------------------------------------
	UFUNCTION()
	virtual bool CanFire();
	
	// Fire只会在Server调用，SimulateFire会在Server和Client调用
	UFUNCTION()
	virtual void FireStart(const FVector& HitTarget);
	UFUNCTION()
	virtual void FireStop();

	UFUNCTION()
	virtual void LocalFire();

	UFUNCTION(BlueprintCallable)
	virtual void SimulateFire();

	virtual void InteractStart(AActor* InteractActor) override;
	virtual void InteractStop(AActor* InteractActor) override;

	UFUNCTION()
	virtual	void Pickup(AMyCharacter* PickCharacter);

	// Reload和ReloadFinish只会在Server调用
	UFUNCTION()
	virtual	void ReloadStart();
	UFUNCTION()
	virtual	void ReloadStop();

	UFUNCTION(BlueprintCallable)
	void SimulateReload();

	UFUNCTION()
	void ReloadFinished();

	UFUNCTION()
	virtual	void Equip();

	// 丢弃武器
	UFUNCTION()
	virtual void Drop();

	// 设置各种状态下武器的属性
	UFUNCTION()
	void OnEquipped();
	UFUNCTION()
	void OnDropped();
	UFUNCTION()
	void OnIdled();

	void SpendRound();

	virtual void OnRep_Owner() override;
//--------------------------------------------Set && Get-------------------------------------------------------------
	virtual void SetInteractEffectVisibility(bool bVisibility) override;

	void SetAmmo(int32 AmmoToSet);
	void SetHUDWeaponAmmo();

	void SetWeaponState(EWeaponState State);

	// 武器状态改变时其应发生的事
	UFUNCTION()
	virtual void OnStateChanged();

	UFUNCTION(BlueprintCallable)
	void SetSphereCollision(bool bCanOverlapWithPawn);

	void SetMeshSimulatePhysics(bool bSimulatePhysics);

	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return MeshComponent; }

	FORCEINLINE bool IsRangedWeapon() const { return bIsRangedWeapon; }

	FORCEINLINE float GetFireDelay() const { return FireDelay; }

	FORCEINLINE bool CanAutomaticFire() const { return bCanAutomaticFire; }

	FORCEINLINE bool CanReload() const { return bCanReload; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

//--------------------------------------------Callbacks-------------------------------------------------------------
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;

	UPROPERTY(EditAnywhere, Category = Parameter)
	EWeaponType WeaponType;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = Parameter)
	EWeaponState WeaponState;
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bIsRangedWeapon = true;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float FireDelay = 0.1f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bCanAutomaticFire = true;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bCanReload = true;

	// 还未收到的Server发来的更新Ammo的RPC数
	int32 AmmoSequence = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, Category = Ammo)
	int32 Ammo = 30;
	UFUNCTION()
	void OnRep_Ammo();
	//UPROPERTY(EditAnywhere, Category = Ammo)
	//int32 Ammo = 30; // 不复制的版本，采用RPC更新,Client-Side Prediction

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 MaxAmmo = 30;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 AmmoCostPerFire = 1;

private:
//--------------------------------------------Components-------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* PickupWidgetComponent;

};
