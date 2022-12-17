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

//------------------------------------------------Functions------------------------------------------------------------
	float CalculateReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	// 这个函数只会在Server调用
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	// 在Server和Client都会调用
	UFUNCTION()
	void Eliminate(AActor* DamageActor, AController* InstigatorController, AActor* DamageCauser);

	void EliminateTimerFinished();
	
//------------------------------------------------Set&&Get------------------------------------------------------------
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	void SetHealth(float NewHealth);
	void IncreaseHealth(float HealthIncrement);

	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	void SetShield(float NewShield);
	void IncreaseShield(float ShieldIncrement);
	
	bool IsHUDVaild();
	void UpdateHUD(AMyHUD* PlayerHUD);
	void SetHUDHealth();
	void SetHUDShield();
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);

	void SetEliminatedCollision();

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	bool IsTeammate(AController* OtherPlayerController);

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

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = Attribute)
	float Shield = 100.f; // 防弹衣用，后续改成打中人物上身减免部分伤害
	UFUNCTION()
	void OnRep_Shield(float OldShield);

	FTimerHandle EliminateTimer;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float EliminateDelay = 3.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bUseTeammateDamage = false;
//------------------------------------------------Animations--------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Animations)
	UAnimMontage* EliminateMontage;
};
