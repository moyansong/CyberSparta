// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	UFUNCTION()
	virtual void CrouchStart();
	UFUNCTION()
	virtual void CrouchStop();

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
	void Initialized();// ???????????????????????????Tick??????????????????Tick???BeginPlay???
	void OnControllerInitialized();// ??????????????????Controller?????????
	void OnPlayerStateInitialized();// ??????????????????PlayerState?????????

	void SimProxiesTurn();

	UFUNCTION(BlueprintCallable)
	void HideCharacterIfCameraClose();

	UFUNCTION()
	void SimulateHit(const FRotator& HitDirection = FRotator::ZeroRotator, const FVector& HitLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable)
	void PlayHitReactMontage(FName SectionName);

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	// ??????/??????
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

	UFUNCTION(BlueprintCallable)
	void Test();

//------------------------------------------Set&&Get----------------------------------------------------------
	void SetOverlappingActor(AActor* Actor); // ?????????Server??????

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	
	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintCallable)
	bool IsAiming() const;

	float GetHealth() const;

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon() const;

	UFUNCTION(BlueprintCallable)
	FVector GetHitTarget() const;

	UFUNCTION(BlueprintCallable)
	ECombatState GetCombatState() const;

	FORCEINLINE bool ShouldMulticastEffect() const { return bShouldMulticastEffect; }

	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	void SetDisableGameplay(bool DisableGameplay);

	void SetShouldMulticastEffect(bool ShouldMulticastEffect);

	void SetTeam(ETeam Team);

	void SetMaxWalkSpeed(bool bIsRunning);

	ETeam GetTeam() const;

	// ??????Team???????????????
	void SetSpawnLocation();

	void SetAnimationClass();
//--------------------------------------------RPC---------------------------------------------------------------
	UFUNCTION(Server, Reliable)
	void ServerRunStart();

	UFUNCTION(Server, Reliable)
	void ServerRunStop();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit(const FRotator& HitDirection = FRotator::ZeroRotator, const FVector_NetQuantize& HitLocation = FVector::ZeroVector);

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	// ????????????
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainTheLead();
	// ??????????????????
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

//-----------------------------------------Components---------------------------------------------------------
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
private:
	// ????????????, ???LagCompensationComponent??????Server-Side Rewind????????????
	// Box????????????Scale?????????BoxExtent
	// ???????????????????????????HitBox
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

	// ???????????????Widget?????????NetRole...
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UWidgetComponent* OverHeadWidgetComponent;

	// ??????????????????????????????????????????????????????
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UCombatComponent* CombatComponent;

	// ?????????????????????????????????????????????????????????
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UAttributeComponent* AttributeComponent;

	// ??????Buff
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Component)
	UBuffComponent* BuffComponent;

	// ????????????????????????
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
	
	// ??????????????????Actor?????????????????????
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingActor)
	AActor* OverlappingActor; 
	UFUNCTION()
	void OnRep_OverlappingActor(AActor* LastActor);// LastActor???OverlappingActor?????????????????????

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	float TurnRateGamepad;

	// ??????????????????????????????????????????????????????
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float CameraThreshold = 200.f; 

	// ?????????????????????????????????????????????Ping??????
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Net)
	bool bShouldMulticastEffect = true; 

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	bool bLeftGame = false;

	// ?????????????????????????????????
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

public:
	float CurrMaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float AimingJogSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float RunSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float AimingWalkSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Move)
	float CrouchWalkSpeed = 250.f;

//------------------------------------------Delegates----------------------------------------------------------
	FOnLeftGame OnLeftGame;
};
