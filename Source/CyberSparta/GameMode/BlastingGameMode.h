// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "BlastingGameMode.generated.h"

class ABlastingBomb;
class ABlastingZone;

UCLASS()
class CYBERSPARTA_API ABlastingGameMode : public ATeamGameMode
{
	GENERATED_BODY()

//-----------------------------------------------Functions-----------------------------------------------------
public:
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController) override;

	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasSettled() override;

	void DistributeBlastingBomb();

//-----------------------------------------------Parameters-----------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Parameter)
	TSubclassOf<ABlastingBomb> BlastingBombClass;

	ABlastingBomb* BlastingBomb;

	UPROPERTY(EditAnywhere, Category = Parameter)
	TSubclassOf<ABlastingZone> BlastingZoneClass;

	ABlastingZone* BlastingZone;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FVector BlastingZoneCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float BlastingZoneRadius = 1000.f;

	FTimerHandle BlastingTimer;
};
