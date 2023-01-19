// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "../../Types/Team.h"
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

	void UpdateTeamScore(ETeam Team);

//-----------------------------------------------Parameters-----------------------------------------------------
	UPROPERTY(Replicated)
	TArray<AMyPlayerState*> TopScoringPlayers;

	float TopScore = 0.f;

	TArray<AMyPlayerState*> RedTeam;
	TArray<AMyPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_BlueTeamScore();
};
