// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "Net/UnrealNetwork.h"
#include "../PlayerStates/MyPlayerState.h"

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, TopScoringPlayers);
}

void AMyGameState::UpdateTopScore(AMyPlayerState* ScoringPlayerState)
{
	float PlayerScore = ScoringPlayerState->GetScore();
	if (TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(ScoringPlayerState);
		TopScore = PlayerScore;
	}
	else if (PlayerScore == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayerState);
	}
	else if (PlayerScore > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayerState);
		TopScore = PlayerScore;
	}
}
