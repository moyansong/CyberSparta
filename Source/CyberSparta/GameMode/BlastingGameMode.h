// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamGameMode.h"
#include "BlastingGameMode.generated.h"

class ABlastingBomb;

UCLASS()
class CYBERSPARTA_API ABlastingGameMode : public ATeamGameMode
{
	GENERATED_BODY()

//-----------------------------------------------Functions-----------------------------------------------------
public:
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	virtual void HandleMatchHasStarted() override;

//-----------------------------------------------Functions-----------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Parameter)
	TSubclassOf<ABlastingBomb> BlastingBombClass;
};
