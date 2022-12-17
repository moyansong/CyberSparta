// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../GameStates/MyGameState.h"
#include "../PlayerController/MyPlayerController.h"
#include "../PlayerStates/MyPlayerState.h"

ATeamGameMode::ATeamGameMode()
{
	bIsTeamMatch = true;
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	if (MyGameState)
	{
		AMyPlayerState* PlayerState = NewPlayer->GetPlayerState<AMyPlayerState>();
		if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (MyGameState->BlueTeam.Num() >= MyGameState->RedTeam.Num())
			{
				MyGameState->RedTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				MyGameState->BlueTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	AMyPlayerState* PlayerState = Exiting->GetPlayerState<AMyPlayerState>();
	if (MyGameState && PlayerState)
	{
		if (MyGameState->RedTeam.Contains(PlayerState))
		{
			MyGameState->RedTeam.Remove(PlayerState);
		}
		else if (MyGameState->BlueTeam.Contains(PlayerState))
		{
			MyGameState->BlueTeam.Remove(PlayerState);
		}
	}
}

void ATeamGameMode::PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* AttackerController, AMyPlayerController* VictimController)
{
	Super::PlayerEliminated(ElimmedCharacter, AttackerController, VictimController);

	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	AMyPlayerState* AttackerPlayerState = AttackerController ? Cast<AMyPlayerState>(AttackerController->PlayerState) : nullptr;
	if (MyGameState && AttackerPlayerState)
	{
		MyGameState->UpdateTeamScore(AttackerPlayerState->GetTeam());
	}
}

void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	if (MyGameState)
	{
		for (auto State : MyGameState->PlayerArray)
		{
			AMyPlayerState* PlayerState = Cast<AMyPlayerState>(State.Get());
			if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (MyGameState->BlueTeam.Num() >= MyGameState->RedTeam.Num())
				{
					MyGameState->RedTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					MyGameState->BlueTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

void ATeamGameMode::HandleMatchHasSettled()
{
	Super::HandleMatchHasSettled();
}
