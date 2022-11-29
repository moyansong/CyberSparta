// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, AController*, InstigatorController, AActor*, DamageCauser, AActor*, DamageActor, UAttributeComponent*, OwningCopmonent, float, Damage, float, NewHealth);

class AMyCharacter;
class AMyPlayerController;
class AMyHUD;
class UAnimMontage;
class UAttributeWidget;

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
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	void SetHealth(float NewHealth);
	void IncreaseHealth(float HealthIncrement);

	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	void SetShield(float NewShield);
	void IncreaseShield(float ShieldIncrement);

	// 这个函数只会在Server调用
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUD(AMyHUD* PlayerHUD);
	void SetHUDHealth();
	void SetHUDShield();
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);

	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	// 在Server和Client都会调用
	UFUNCTION()
	void Elim(AActor* DamageActor, AController* InstigatorController, AActor* DamageCauser);

	void ElimTimerFinished();

	bool IsHUDVaild();

private:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;
	UPROPERTY()
	AMyHUD* MyHUD;
	UPROPERTY()
	UAttributeWidget* AttributeWidget;
	
	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, Category = Attribute)
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = Attribute)
	float Shield = 100.f; // 防弹衣用，后续改成打中人物上身减免部分伤害
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	FTimerHandle ElimTimer;

	UPROPERTY(EditAnywhere)
	float ElimDelay = 3.f;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChangedSignature OnHealthChanged;
//------------------------------------------------Animations--------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Animations)
	UAnimMontage* ElimMontage;
};
