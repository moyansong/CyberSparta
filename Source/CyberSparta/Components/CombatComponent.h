// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../HUD/MyHUD.h"
#include "CombatComponent.generated.h"

class UMyCharacter;
class AWeapon;
class UAnimMontage;
class AMyPlayerController;
class AMyHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSPARTA_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
public:	
	UCombatComponent();
	friend class AMyCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


//-----------------------------------------------Functions------------------------------------------------------------
	UFUNCTION()
	void EquipWeaponStart(AWeapon* Weapon);
	UFUNCTION()
	void EquipWeaponStop(AWeapon* Weapon);

	UFUNCTION()
	void FireStart();
	UFUNCTION()
	void FireStop();
	UFUNCTION()
	void Fire();

	// 该函数只模拟开火，生成动画特效之类的，不生成子弹,会广播到客户端
	UFUNCTION(BlueprintCallable)
	void SimulateFire();

	void SetIsAiming(bool bAiming);

	// 穿过准星的射线
	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION()
	void TargetStart();
	UFUNCTION()
	void TargetStop();

	FORCEINLINE bool CanFire() const { return bCanFire; }

	void Test();
//-----------------------------------------------RPC------------------------------------------------------------
	// 由于击中位置是客户端检测的，所以需要传递给服务器，因为子弹再服务器生成，FVector_NetQuantize是网络中传参用的，只能传整数
	UFUNCTION(Server, Reliable)
	void ServerFireStart(const FVector_NetQuantize& HitTarget); 

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireStart(const FVector_NetQuantize& HitTarget);

private:
//------------------------------------------------Parameters--------------------------------------------------------------
	AMyCharacter* MyCharacter;
	AMyPlayerController* MyController;
	AMyHUD* MyHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(Replicated)
	bool bIsAiming = true;

	// 准星扩大
	float CrosshairsInAirFactor;
	float CrosshairsFireFactor;

	// 准星指向的坐标
	FVector MyHitTarget;

	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;

	bool bCanFire = true;

};
