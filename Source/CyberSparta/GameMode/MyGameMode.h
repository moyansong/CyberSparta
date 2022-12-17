// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MyGameMode.generated.h"

class AMyCharacter;
class AMyPlayerController;
class AMyPlayerState;
class AMyGameState;

namespace MatchState
{
	extern CYBERSPARTA_API const FName Settlement; // Before WaitingPostMatch,Show some Widget
}

UCLASS()
class CYBERSPARTA_API AMyGameMode : public AGameMode
{
	GENERATED_BODY()

//-----------------------------------------------Functions-----------------------------------------------------
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

	// 结算阶段
	virtual void HandleMatchHasSettled();
public:
	AMyGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void SetMatchState(FName NewState) override;

	// 玩家淘汰
	virtual void PlayerEliminated(AMyCharacter* ElimmedCharacter, AMyPlayerController* AttackerController, AMyPlayerController* VictimController);

	// 重生
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(AMyPlayerState* PlayerState);

	void UpdatePlayerState(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState);
	void UpdateGameState(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState);

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
//-----------------------------------------------Parameters-----------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = Time)
	float WarmupTime = 5.f; // 过几秒再Spawn Pawn

	UPROPERTY(EditDefaultsOnly, Category = Time)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = Time)
	float SettlementTime = 30.f; // 结算面板显示的时间

	float LevelStartingTime = 0.f; // 进入Level花的时间，进入Level后开始Warmup

protected:
	UPROPERTY()
	AMyGameState* MyGameState;

	UPROPERTY(EditDefaultsOnly)
	float CountdownTime = 0.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bIsTeamMatch = false;
};
