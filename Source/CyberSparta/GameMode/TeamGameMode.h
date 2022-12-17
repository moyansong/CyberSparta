// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameMode.h"
#include "TeamGameMode.generated.h"

class AMyCharacter;
class AMyPlayerController;

UCLASS()
class CYBERSPARTA_API ATeamGameMode : public AMyGameMode
{
	GENERATED_BODY()

//-----------------------------------------------Functions-----------------------------------------------------
public:
	ATeamGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* AttackerController, AMyPlayerController* VictimController);
protected:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasSettled() override;
//-----------------------------------------------Parameters-----------------------------------------------------

};
