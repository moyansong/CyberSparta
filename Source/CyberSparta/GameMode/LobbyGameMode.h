// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 关卡蓝图中调用出现打包错误Please refresh node or break links to remove pin？
	UFUNCTION(BlueprintCallable)
	void LobbySetup(FString GameMapPath = FString(TEXT("")), int32 MaxPlayerNum = 2);

private:
//-----------------------------------------Parameter-------------------------------------------------------------
	FString PathOfGameMap{ TEXT("") };// 游戏地图
	int32 MaxNumOfPlayers{ 2 }; // 当前玩家数等于他时开始游戏
};
