// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class AMyCharacter;
class AMyPlayerController;
class AMyHUD;
class ABuff;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSPARTA_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AMyCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
//---------------------------------------------Functions-------------------------------------------------------------
	void AddBuff(ABuff* Buff);

	UFUNCTION(Server, Reliable)
	void ServerAddBuff(ABuff* Buff);

	void RemoveBuff(ABuff* Buff);

protected:
	virtual void BeginPlay() override;

//---------------------------------------------Parameters-------------------------------------------------------------

private:
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;
	UPROPERTY()
	AMyHUD* MyHUD;

	UPROPERTY(Replicated)
	TArray<ABuff*> Buffs;
};
