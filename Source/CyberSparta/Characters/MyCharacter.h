// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UUserWidget;
class UAudioComponent;
class USoundCue;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAttributeComponent;
class AMyPlayerController;

UCLASS()
class CYBERSPARTA_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

//-------------------------------------------Input--------------------------------------------------------------
	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	UFUNCTION()
	void TurnAtRate(float Rate);

	UFUNCTION()
	void LookUpAtRate(float Rate);

	UFUNCTION()
	virtual void JumpStart();
	UFUNCTION()
	virtual void JumpStop();

	UFUNCTION()
	virtual void CrouchStart();
	UFUNCTION()
	virtual void CrouchStop();

public:
	UFUNCTION()
	virtual void EquipWeaponStart();
	UFUNCTION()
	virtual void EquipWeaponStop();

	UFUNCTION()
	virtual void FireStart();
	UFUNCTION()
	virtual void FireStop();

	UFUNCTION()
	virtual void SetIsAiming(bool bAiming = true);

	UFUNCTION()
	void TargetStart();
	UFUNCTION()
	void TargetStop();

	UFUNCTION()
	void InteractStart();
	UFUNCTION()
	void InteractStop();

private:
//-----------------------------------------Components---------------------------------------------------------
	// Camera boom positioning the camera behind the character
	// AllowPrivateAccess = "true"将私有变量公开给蓝图
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	USpringArmComponent* CameraBoom;

	// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	UCameraComponent* FollowCamera;

	// 人物头顶的Widget，展示NetRole...
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Widget")
	UWidgetComponent* OverHeadWidgetComponent;

	// 人物所有有关攻击的东西交由该组件完成
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UCombatComponent* CombatComponent;

	// 负责人物的各种属性，例如生命值，怒气等
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UAttributeComponent* AttributeComponent;

protected:
//------------------------------------------Parameters----------------------------------------------------------
	AMyPlayerController* MyController;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingActor)
	AActor* OverlappingActor; // 与角色重叠的Actor，可能发生互动
	UFUNCTION()
	void OnRep_OverlappingActor(AActor* LastActor);// LastWeapon是OverlappingWeapon还未复制时的值

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	float TurnRateGamepad;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float CameraThreshold = 200.f; // 相机距离人距离小于这个就会把人物隐藏

	float CurrMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	float AimingJogSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	float RunSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	float AimingWalkSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	float CrouchWalkSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Net")
	bool bShouldMulticastFire = true; // 是否广播开火的动画和特效

//------------------------------------------RPC----------------------------------------------------------
	UFUNCTION(Server, Reliable)
	void ServerEquipWeaponStart();

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bAiming);

public:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

//------------------------------------------Functions----------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquipped();

	UFUNCTION(BlueprintCallable)
	bool IsAiming();

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	UFUNCTION(BlueprintCallable)
	void HideCharacterIfCameraClose();

	UFUNCTION(BlueprintCallable)
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	// 死亡/淘汰
	void Elim();
	
	UFUNCTION(BlueprintCallable)
	void Test();

//------------------------------------------Set&&Get----------------------------------------------------------
	void SetOverlappingActor(AActor* Actor);

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon();

	UFUNCTION(BlueprintCallable)
	FVector GetHitTarget() const;

	FORCEINLINE bool ShouldMulticastFire() const { return bShouldMulticastFire; }

	void SimProxiesTurn();

};
