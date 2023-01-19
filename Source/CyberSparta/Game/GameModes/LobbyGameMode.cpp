// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 PlayerNum = GameState.Get()->PlayerArray.Num();
	if (PlayerNum == MaxNumOfPlayers)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString(TEXT("/Game/Maps/PVP/TestPVPMap?listen")));// 会将所有客户端都传送到新地图
		}
	}
}

void ALobbyGameMode::LobbySetup(FString GameMapPath, int32 MaxPlayerNum)
{
	PathOfGameMap = GameMapPath;
	MaxNumOfPlayers = MaxPlayerNum;
}
