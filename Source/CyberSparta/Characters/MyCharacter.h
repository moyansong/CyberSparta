// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterTypes.h"
#include "../Types/CombatState.h"
#include "../Types/Team.h"
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
class ULagCompensationComponent;
class UBuffComponent;
class AMyPlayerController;
class AMyPlayerState;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UMaterialInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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
	virtual void Destroyed() override;

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

	virtual bool CanJumpInternal_Implementation() const override;

	UFUNCTION()
	virtual void CrouchStart();
	UFUNCTION()
	virtual void CrouchStop();

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void UnCrouch(bool bClientSimulation = false) override;

public:
	UFUNCTION()
	virtual void EquipWeapon(float Value);

	UFUNCTION()
	virtual void FireStart();
	UFUNCTION()
	virtual void FireStop();

	UFUNCTION()
	void TargetStart();
	UFUNCTION()
	void TargetStop();

	UFUNCTION()
	void InteractStart();
	UFUNCTION()
	void InteractStop();

	UFUNCTION()
	void ReloadStart();
	UFUNCTION()
	void ReloadStop();

	UFUNCTION()
	void ThrowWeaponStart();
	UFUNCTION()
	void ThrowWeaponStop();

	UFUNCTION()
	void RunStart();
	UFUNCTION()
	void RunStop();

//------------------------------------------Functions---------------------------------------------------------
	void Initialized();// 初始化一些数据，在Tick里执行，因为Tick在BeginPlay前
	void OnControllerInitialized();// 在第一次获得Controller时执行
	void OnPlayerStateInitialized();// 在第一次获得PlayerState时执行

	void SimProxiesTurn();

	UFUNCTION(BlueprintCallable)
	void HideCharacterIfCameraClose();

	UFUNCTION()
	void SimulateHit(const FVector& HitLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable)
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	// 死亡/淘汰
	void Eliminate();

	UFUNCTION()
	void SimulateEliminate();

	UFUNCTION(BlueprintCallable)
	void UpdateHUD(AMyHUD* PlayerHUD);

	UFUNCTION()
	void KillReward();

	UFUNCTION()
	void OnPingIsTooHigh(float Ping);

	void LeaveGame();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void ChangeWeapon(int32 Value);

	UFUNCTION(BlueprintCallable)
	void Test();

//------------------------------------------Set && Get----------------------------------------------------------
	void SetOverlappingActor(AActor* Actor); // 只会在Server进行

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }

	float GetHealth() const;

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable)
	bool IsAiming() const;

	UFUNCTION(BlueprintCallable)
	FVector GetHitTarget() const;

	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon() const;

	UFUNCTION(BlueprintCallable)
	ECombatState GetCombatState() const;

	void SetDisableGameplay(bool DisableGameplay);
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	void SetShouldMulticastEffect(bool ShouldMulticastEffect);
	FORCEINLINE bool ShouldMulticastEffect() const { return bShouldMulticastEffect; }

	ETeam GetTeam() const;
	void SetTeam(ETeam Team);

	// 根据Team选择重生点
	void SetSpawnLocation();
	
	// 可以根据武器类型改变动画蓝图类
	// 已废弃，所有动画采用一个动画蓝图实现
	void SetAnimationClass();

	void SetMaxWalkSpeed(bool bIsRunning);

	void SetHitDirection(const FVector& HitLocation);
	FORCEINLINE EHitDirection GetHitDirection() const { return HitDirection; }

//--------------------------------------------RPC---------------------------------------------------------------
	UFUNCTION(Server, Reliable)
	void ServerRunStart();

	UFUNCTION(Server, Reliable)
	void ServerRunStop();

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	// 得分领先
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainTheLead();
	// 得分不再领先
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit(const FVector_NetQuantize& HitLocation = FVector::ZeroVector);

//-----------------------------------------Components---------------------------------------------------------
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
private:
	// 对应骨骼, 在LagCompensationComponent里做Server-Side Rewind精确命中
	// Box不要改变Scale，要改BoxExtent
	// 这些用自定义的通道HitBox
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;
	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	UCameraComponent* FollowCamera;

	// 人物头顶的Widget，展示NetRole...
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UWidgetComponent* OverheadWidgetComponent;

	// 人物所有有关攻击的东西交由该组件完成
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UCombatComponent* CombatComponent;

	// 负责人物的各种属性，例如生命值，怒气等
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UAttributeComponent* AttributeComponent;

	// 处理Buff
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UBuffComponent* BuffComponent;

	// 处理与延迟相关的
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	ULagCompensationComponent* LagCompensationComponent;

	UPROPERTY(EditAnywhere, Category = Animation)
	TSubclassOf<UAnimInstance> DefaultAnimationClass;

//------------------------------------------Parameters----------------------------------------------------------	
protected:
	UPROPERTY()
	AMyPlayerController* MyController;
	UPROPERTY()
	AMyPlayerState* MyPlayerState;
	
	// 与角色重叠的Actor，可能发生互动
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingActor)
	AActor* OverlappingActor; 
	UFUNCTION()
	void OnRep_OverlappingActor(AActor* LastActor);// LastActor是OverlappingActor还未复制时的值

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	float TurnRateGamepad;

	// 相机与人的距离小于这个就会把人物隐藏
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float CameraThreshold = 200.f; 

	bool bLeftGame = false;

	UPROPERTY()
	bool bDisableGameplay = false;

	// 是否广播人物的动画和特效，根据Ping决定
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Net)
	bool bShouldMulticastEffect = true; 

	// 得分最高的人显示的特效
	UPROPERTY(EditAnywhere, Category = Emitter)
	UNiagaraSystem* CrownSystem;
	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInstance* OriginalMaterial;
	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInstance* BlueTeamMaterial;
	UPROPERTY(EditAnywhere, Category = Material)
	UMaterialInstance* RedTeamMaterial;

	// 人物被打的方向
	UPROPERTY(ReplicatedUsing = OnRep_HitDirection, VisibleAnywhere, Category = Parameter)
	EHitDirection HitDirection = EHitDirection::EHD_Forward;
	UFUNCTION()
	void OnRep_HitDirection(EHitDirection LastHitDirection);

public:
	float CurrMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float RunSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float AimingJogSpeed = 650.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float AimingWalkSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float CrouchWalkSpeed = 300.f;

//------------------------------------------Delegates----------------------------------------------------------
	FOnLeftGame OnLeftGame;

};
