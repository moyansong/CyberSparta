// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

class AMyCharacter;
class AMyPlayerController;

UCLASS()
class CYBERSPARTA_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* VictimController, AMyPlayerController* AttackController);
};
