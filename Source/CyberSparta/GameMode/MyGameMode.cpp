// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController/MyPlayerController.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../GameStates/MyGameState.h"

namespace MatchState
{
	const FName Settlement = FName("Settlement");
}

AMyGameMode::AMyGameMode()
{
	// 不会立刻Spawn Pawn
	bDelayedStart = true;
}

void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->OnMatchStateSet(MatchState, bIsTeamMatch);
		}
	}
}

void AMyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Settlement);
		}
	}
	else if (MatchState == MatchState::Settlement)
	{
		CountdownTime = WarmupTime + MatchTime + SettlementTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();// 只有打包的游戏或者Launch Game里才能生效，编辑器里没效果，LaunchGame里可以用命令行输入Servertravel xxMap,因为设定了人数不够不会传送到对战地图
		}
	}
}

void AMyGameMode::PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* AttackerController, AMyPlayerController* VictimController)
{
	if (!ElimmedCharacter || !AttackerController || !VictimController) return;
	AMyPlayerState* AttackerPlayerState = Cast<AMyPlayerState>(AttackerController->PlayerState);
	AMyPlayerState* VictimPlayerState = Cast<AMyPlayerState>(VictimController->PlayerState);

	if (AttackerPlayerState && VictimPlayerState)
	{
		UpdatePlayerState(AttackerPlayerState, VictimPlayerState);
		UpdateGameState(AttackerPlayerState, VictimPlayerState);
	}
}

void AMyGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset(); // 保存PlayerState
		ElimmedCharacter->Destroy();
	}
	
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Index = FMath::RandRange(0, PlayerStarts.Num() - 1);
		// 注意：重新生成的Pawn在BeginPlay完成后才会交给Controller，所以在构造和BeginPlay里关于Controller的逻辑要写到PlayerController::OnPossess里
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Index]); 
	}
}

void AMyGameMode::UpdatePlayerState(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState)
{
	if (!AttackerPlayerState || !VictimPlayerState) return;

	if (AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->IncreaseScore(1.f);
	}
	VictimPlayerState->IncreaseDefeats(1); 
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMyPlayerController* PlayerController = Cast<AMyPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->BroadcastKill(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void AMyGameMode::UpdateGameState(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState)
{
	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	if (!MyGameState || !AttackerPlayerState || !VictimPlayerState) return;

	TArray<AMyPlayerState*> PreLeadPlayers = MyGameState->TopScoringPlayers;
	MyGameState->UpdateTopScore(AttackerPlayerState);
	if (MyGameState->TopScoringPlayers.Contains(AttackerPlayerState))
	{
		AMyCharacter* LeadCharacter = Cast<AMyCharacter>(AttackerPlayerState->GetPawn());
		if (LeadCharacter)
		{
			LeadCharacter->MulticastGainTheLead();
		}
	}
	for (int32 i = 0; i < PreLeadPlayers.Num(); ++i)
	{
		if (!MyGameState->TopScoringPlayers.Contains(PreLeadPlayers[i]))
		{
			AMyCharacter* LostLeadCharacter = Cast<AMyCharacter>(PreLeadPlayers[i]->GetPawn());
			if (LostLeadCharacter)
			{
				LostLeadCharacter->MulticastLostTheLead();
			}
		}
	}
}

void AMyGameMode::PlayerLeftGame(AMyPlayerState* PlayerState)
{
	if (!PlayerState) return;
	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	if (MyGameState && MyGameState->TopScoringPlayers.Contains(PlayerState))
	{
		MyGameState->TopScoringPlayers.Remove(PlayerState);
	}
	AMyCharacter* LeavingCharacter = Cast<AMyCharacter>(PlayerState->GetPawn());
}