// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, AController*, InstigatorController, AActor*, DamageCauser, AActor*, DamageActor, UAttributeComponent*, OwningCopmonent, float, Damage, float, NewHealth);

class AMyCharacter;
class AMyPlayerController;
class AMyHUD;

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
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUD();

	UFUNCTION(BlueprintCallable)
	bool IsAlive();
private:
//------------------------------------------------Parameters--------------------------------------------------------------
	AMyCharacter* MyCharacter;
	AMyPlayerController* MyController;
	AMyHUD* MyHUD;
	
	UPROPERTY(EditAnywhere, Category = Attribute)
	float MaxHealth = 1000.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = Attribute)
	float Health = 1000.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(BlueprintAssignable)
	FOnHealthChangedSignature OnHealthChanged;
};
