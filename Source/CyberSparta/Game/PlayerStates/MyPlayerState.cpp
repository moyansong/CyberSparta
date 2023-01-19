// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Components/AttributeComponent.h"
#include "../PlayerControllers/MyPlayerController.h"

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, Defeats);
	DOREPLIFETIME(AMyPlayerState, HeadShots);
	DOREPLIFETIME(AMyPlayerState, Team);
}

void AMyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		// 能获得Controller就代表是本地控制的
		if (MyController)
		{
			MyController->SetHUDScore(GetScore()); 
		}
	}
}

void AMyPlayerState::IncreaseScore(float ScoreIncrement)
{
	SetScore(GetScore() + ScoreIncrement);
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDScore(GetScore());
		}
	}
}

void AMyPlayerState::OnRep_Defeats()
{
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDDefeats(Defeats);
		}
	}
}

void AMyPlayerState::IncreaseDefeats(int32 DefeatsIncrement)
{
	Defeats += DefeatsIncrement;
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDDefeats(Defeats);
		}
	}
}

void AMyPlayerState::OnRep_HeadShots()
{
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDHeadShots(HeadShots);
		}
	}
}

void AMyPlayerState::IncreaseHeadShots(int32 HeadShotsIncrement)
{
	HeadShots += HeadShotsIncrement;
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		if (!MyController) MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyController)
		{
			MyController->SetHUDHeadShots(HeadShots);
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

	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyCharacter->SetTeam(Team);
	}
}

void AMyPlayerState::OnRep_Team()
{
	if (!MyCharacter) MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyCharacter->SetTeam(Team);
	}
}
