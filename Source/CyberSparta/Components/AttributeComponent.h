// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, AActor*, DamageActor, float, OldHealth, float, NewHealth);

class AMyCharacter;
class AMyPlayerController;
class AMyHUD;
class UAnimMontage;
class UAttributeWidget;
class AMyGameMode;
class AMyPlayerState;

USTRUCT(BlueprintType)
struct FDamage
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHeadShot;

	UPROPERTY()
	float TotalDamage;

	UPROPERTY()
	float DamageToHealth;

	UPROPERTY()
	float DamageToShield;
};

// 任何需要初始化在HUD里的东西，都要在以下几个接口设置
// 1.MyPlayerController::InitializeInProgrossWidget;
// 2.AMyCharacter::Initialized();
// 3.变量复制的地方

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSPARTA_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
public:	
	friend class AMyCharacter;
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//------------------------------------------------Set&&Get------------------------------------------------------------
	void SetHealth(float NewHealth);
	void IncreaseHealth(float HealthIncrement);
	void DecreaseHealth(float HealthDecrement);
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	void SetShield(float NewShield);
	void IncreaseShield(float ShieldIncrement);
	void DecreaseShield(float ShieldDecrement);
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float GetShieldDamageReduction() const { return ShieldDamageReduction; }

	bool IsHUDVaild();
	void SetHUDHealth();
	void SetHUDShield();
	void SetHUDAttribute();
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDHeadShots(int32 HeadShots);

	void SetEliminatedCollision();

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	bool IsTeammate(AController* OtherPlayerController);

//------------------------------------------------Functions------------------------------------------------------------
	FDamage CalculateReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	float CalculateShiledReceivedDamage(float Damage);

	// 这个函数只会在Server调用
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	// 在Server和Client都会调用
	UFUNCTION()
	void Eliminate(AActor* DamageActor, AController* InstigatorController, AActor* DamageCauser, bool bHeadShot = false);

	void EliminateTimerFinished();
	
	void UpdateHUD(AMyHUD* PlayerHUD);

//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(BlueprintAssignable)
	FOnHealthChangedSignature HealthChangedDelegate;

private:
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;
	UPROPERTY()
	AMyHUD* MyHUD;
	UPROPERTY()
	UAttributeWidget* AttributeWidget;
	UPROPERTY()
	AMyGameMode* MyGameMode;
	UPROPERTY()
	AMyPlayerState* MyPlayerState;
	
	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, Category = Attribute)
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxShield = 100.f;

	// 防弹衣，打中躯干部分(上半身除了头和胳膊部分)可以减伤
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = Attribute)
	float Shield = 100.f; 
	UFUNCTION()
	void OnRep_Shield(float OldShield);

	// 防弹衣减伤比例
	UPROPERTY(EditAnywhere, Category = Attribute)
	float ShieldDamageReduction = 0.4f;

	FTimerHandle EliminateTimer;

	// 死亡后重生的等待时间
	UPROPERTY(EditAnywhere, Category = Parameter)
	float EliminateDelay = 3.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bUseTeammateDamage = false;
//------------------------------------------------Animations--------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Animations)
	UAnimMontage* EliminateMontage;
};
