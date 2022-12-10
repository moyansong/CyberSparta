// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "Net/UnrealNetwork.h"
#include "../CyberSparta.h"
#include "../PlayerController/MyPlayerController.h"
#include "../PlayerStates/MyPlayerState.h"

void AMyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, TopScoringPlayers);
	DOREPLIFETIME(AMyGameState, RedTeamScore);
	DOREPLIFETIME(AMyGameState, BlueTeamScore);
}

void AMyGameState::UpdateTopScore(AMyPlayerState* ScoringPlayerState)
{
	float PlayerScore = ScoringPlayerState->GetScore();
	if (PlayerScore < 1.f) return;
	else if (TopScoringPlayers.IsEmpty())
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

void AMyGameState::UpdateTeamScore(ETeam Team)
{
	AMyPlayerController* MyController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (Team == ETeam::ET_BlueTeam)
	{
		BlueTeamScore += 1.f; 
		if (MyController) MyController->SetHUDBlueTeamScore(BlueTeamScore);
	}
	else if (Team == ETeam::ET_RedTeam)
	{
		RedTeamScore += 1.f; 
		if (MyController) MyController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AMyGameState::OnRep_BlueTeamScore()
{
	AMyPlayerController* MyController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyController) MyController->SetHUDBlueTeamScore(BlueTeamScore);
}

void AMyGameState::OnRep_RedTeamScore()
{
	AMyPlayerController* MyController = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyController) MyController->SetHUDRedTeamScore(RedTeamScore); 
}
