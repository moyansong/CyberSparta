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
class UTexture2D;

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
	void EquipWeapon(int32 Value, bool bThrowLastWeapon = false); // 在Server和Client都会调用

	void LocalEquipWeapon();

	// 通过动画通知调用
	// Value == 0表示卸下上把武器，1表示装备新武器，2表示装备完成
	void ChangeWeapon(int32 Value);

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
	UFUNCTION()
	void LocalFire(const FVector& HitTarget);
	UFUNCTION()
	void FireFinished();
	UFUNCTION()
	void LocalFireStop();

	// 该函数只模拟开火，生成动画特效之类的，不生成子弹
	UFUNCTION(BlueprintCallable)
	void SimulateFire();

	// 穿过准星的射线
	void TraceUnderCrosshairs(FHitResult& HitResult);

	void InitializeHUDCrosshairs();

	UFUNCTION()
	void TargetStart();
	UFUNCTION()
	void TargetStop();

	UFUNCTION()
	bool CanReload();
	UFUNCTION()
	void ReloadStart();
	UFUNCTION()
	void ReloadStop();

	UFUNCTION(BlueprintCallable)
	void SimulateReload(); // 在OnRep_CombatState中调用

	UFUNCTION(BlueprintCallable)
	void ReloadFinished(); // 在Montage结束的回调里调用

	void SpawnDefaultWeapons();

	void KillReward();

	UFUNCTION()
	void AddWeapon(AWeapon* Weapon); // 只在Server进行

	UFUNCTION()
	void RemoveWeapon(AWeapon* Weapon);// 不保证只在Server进行

	void UpdateHUD(AMyHUD* PlayerHUD);

	UFUNCTION()
	void Test();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void OnStateChanged();
//---------------------------------------------Set && Get------------------------------------------------------------
	void SetIsAiming(bool bAiming);

	void SetIdleWeapon(AWeapon* Weapon);

	void SetHUDWeapon();

	void SetHUDWeaponAmmo();

	void SetHUDWeaponImage();

	bool IsHUDVaild();

	void SetCombatState(ECombatState State);

	void SetHUDCrosshairs(float DeltaTime);

protected:
	virtual void BeginPlay() override;
//--------------------------------------------------RPC------------------------------------------------------------
	// 由于击中位置是客户端检测的，所以需要传递给服务器，FVector_NetQuantize是网络中传参用的，只能传整数,FVector_NetQuantize100更精准
	// 传FireDelay是防作弊的，因为FireDelay是客户端决定的，在这里查看客户端是否修改了FireDelay
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize100& HitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize100& HitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFireStop();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireStop();

	UFUNCTION(Server, Reliable)
	void ServerReloadStart();

	UFUNCTION(Server, Reliable)
	void ServerAddWeapon(AWeapon* Weapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(int32 Value, bool bThrowLastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerThrowWeapon();

	UFUNCTION(Server, Reliable)
	void ServerSetCombatState(ECombatState State);

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

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon, VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;
	UFUNCTION()
	void OnRep_EquippedWeapon(AWeapon* LastWeapon);

	AWeapon* LastEquippedWeapon;

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

	UPROPERTY(ReplicatedUsing = OnRep_bIsAiming)
	bool bIsAiming = true;
	UFUNCTION()
	void OnRep_bIsAiming();

	float LastFireTime = -100.f;

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
