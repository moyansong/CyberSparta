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
			PlayerController->OnMatchStateSet(MatchState);
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

void AMyGameMode::PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* VictimController, AMyPlayerController* AttackController)
{
	AMyPlayerState* AttackPlayerState = AttackController ? Cast<AMyPlayerState>(AttackController->PlayerState) : nullptr;
	AMyPlayerState* VictimPlayerState = VictimController ? Cast<AMyPlayerState>(VictimController->PlayerState) : nullptr;

	AMyGameState* MyGameState = GetGameState<AMyGameState>();

	if (AttackPlayerState && AttackPlayerState != VictimPlayerState && MyGameState)
	{
		AttackPlayerState->IncreaseScore(1.f);
		MyGameState->UpdateTopScore(AttackPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->IncreaseDefeats(1);
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
