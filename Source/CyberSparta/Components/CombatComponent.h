// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../HUD/MyHUD.h"
#include "../Weapons/WeaponTypes.h"
#include "../Types/CombatState.h"
#include "CombatComponent.generated.h"

class UMyCharacter;
class AWeapon;
class UAnimMontage;
class AMyPlayerController;
class AMyHUD;
class UWeaponWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSPARTA_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	UCombatComponent();
	friend class AMyCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void UninitializeComponent() override;
//-----------------------------------------------Functions------------------------------------------------------------
	UFUNCTION()
	void EquipWeapon(int32 Value); // 在Server和Client都会调用
	
	UFUNCTION()
	void ThrowWeapon();
 
	UFUNCTION()
	bool CanFire();
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

	UFUNCTION()
	void ReloadStart();
	UFUNCTION()
	void ReloadStop();

	UFUNCTION(BlueprintCallable)
	void SimulateReload(); // 在OnRep_CombatState中调用

	UFUNCTION(BlueprintCallable)
	void ReloadFinished(); // 在动画通知中调用，换弹结束后把CombatState设为Idle，现在暂时用定时器实现

	void SpawnDefaultWeapons();

	UFUNCTION()
	void AddWeapon(AWeapon* Weapon); // 只在Server进行

	UFUNCTION()
	void RemoveWeapon(AWeapon* Weapon);// 不保证只在Server进行

	void SetIdleWeapon(AWeapon* Weapon);

	void UpdateHUD(AMyHUD* PlayerHUD);

	void SetHUDWeaponAmmo();

	bool IsHUDVaild();

	void Test();

protected:
	virtual void BeginPlay() override;
//--------------------------------------------------RPC------------------------------------------------------------
	// 由于击中位置是客户端检测的，所以需要传递给服务器，因为子弹再服务器生成，FVector_NetQuantize是网络中传参用的，只能传整数
	UFUNCTION(Server, Reliable)
	void ServerFireStart(const FVector_NetQuantize& HitTarget); 

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireStart(const FVector_NetQuantize& HitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReloadStart();

	UFUNCTION(Server, Reliable)
	void ServerAddWeapon(AWeapon* Weapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(int32 Value);

	UFUNCTION(Server, Reliable)
	void ServerThrowWeapon();

private:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;
	UPROPERTY()
	AMyHUD* MyHUD;
	UPROPERTY()
	UWeaponWidget* WeaponWidget;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(Replicated, EditAnywhere, Category = Weapon)
	TArray<TSubclassOf<AWeapon>> WeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_Weapons)
	TArray<AWeapon*> Weapons;
	UFUNCTION()
	void OnRep_Weapons();

	UPROPERTY(ReplicatedUsing = OnRep_CurrWeaponIndex)
	int32 CurrWeaponIndex = 0;
	UFUNCTION()
	void OnRep_CurrWeaponIndex();

	int32 WeaponsNum = 0;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Idle;
	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(Replicated)
	bool bIsAiming = true;

	bool bCanFire = true;

	UPROPERTY(EditAnywhere, Category = Parameters)
	bool bShouldAutomaticReload = true;

	UPROPERTY(EditAnywhere, Category = Parameters)
	float AutomaticReloadDelay = 5.f;

	// 准星扩大
	float CrosshairsInAirFactor;
	float CrosshairsFireFactor;

	// 准星指向的坐标
	FVector MyHitTarget;

	FHUDPackage HUDPackage;

	FTimerHandle FireTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle AutomaticReloadTimer;

};
