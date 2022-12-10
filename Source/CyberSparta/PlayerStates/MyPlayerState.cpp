// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Components/AttributeComponent.h"
#include "../PlayerController/MyPlayerController.h"

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, Defeats);
	DOREPLIFETIME(AMyPlayerState, Team);
}

void AMyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDScore(GetScore()); 
		}
	}
}

void AMyPlayerState::IncreaseScore(float ScoreIncrement)
{
	SetScore(GetScore() + ScoreIncrement);
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDScore(GetScore());
		}
	}
}

void AMyPlayerState::OnRep_Defeats()
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDDefeats(Defeats);
		}
	}
}

void AMyPlayerState::IncreaseDefeats(int32 DefeatsIncrement)
{
	Defeats += DefeatsIncrement;
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDDefeats(Defeats);
		}
	}
}

bool AMyPlayerState::IsTeammate(AMyPlayerState* OtherPlayer)
{
	return Team != ETeam::ET_NoTeam && OtherPlayer && Team == OtherPlayer->GetTeam();
}

void AMyPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyCharacter->SetTeam(Team);
	}
}

void AMyPlayerState::OnRep_Team()
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyCharacter->SetTeam(Team);
	}
}
