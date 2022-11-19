// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractInterface.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UCombatComponent;
class UTexture2D;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),//枪刚初始化在世界里
	EWS_Equipped UMETA(DisplayName = "Equipped"),//装备在人身上
	EWS_Dropped UMETA(DisplayName = "Dropped"),//被人丢了

	EWS_MAX UMETA(DisplayName = "DefaultMax")
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

//-----------------------------------------------Functions-------------------------------------------------------------
	UFUNCTION()
	virtual void FireStart(const FVector& HitTarget);
	UFUNCTION()
	virtual void FireStop();

	UFUNCTION(BlueprintCallable)
	virtual void SimulateFire();

	UFUNCTION()
	virtual	void EquipWeaponStart();
	UFUNCTION()
	virtual	void EquipWeaponStop();

	virtual void InteractStart(AActor* InteractActor) override;
	virtual void InteractStop(AActor* InteractActor) override;
//--------------------------------------------Set && Get-------------------------------------------------------------
	virtual void SetInteractWidgetVisibility(bool bVisibility) override;

	void SetWeaponState(EWeaponState State);

	// 武器状态改变时其应发生的事
	UFUNCTION(BlueprintCallable)
	virtual void SwitchOfWeaponState();

	UFUNCTION(BlueprintCallable)
	void SetSphereCollision(bool bCanOverlapWithPawn);

	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return MeshComponent; }

	FORCEINLINE bool IsRangedWeapon() const { return bIsRangedWeapon; }

	FORCEINLINE float GetFireDelay() const { return FireDelay; }

	FORCEINLINE bool CanAutomaticFire() const { return bCanAutomaticFire; }

protected:
	virtual void BeginPlay() override;

//--------------------------------------------Callbacks-------------------------------------------------------------
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Parameters")
		EWeaponState WeaponState;
	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Parameters")
		bool bIsRangedWeapon = true;

	UPROPERTY(EditAnywhere, Category = "Parameters")
		float FireDelay = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Parameters")
		bool bCanAutomaticFire = true;

private:
//--------------------------------------------Components-------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* PickupWidgetComponent;

};
