// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

class AMyPlayerState;

UCLASS()
class CYBERSPARTA_API AMyGameState : public AGameState
{
	GENERATED_BODY()

//-----------------------------------------------Functions-----------------------------------------------------
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(AMyPlayerState* ScoringPlayerState);

//-----------------------------------------------Parameters-----------------------------------------------------
	UPROPERTY(Replicated)
	TArray<AMyPlayerState*> TopScoringPlayers;

	float TopScore = 0.f;
};
