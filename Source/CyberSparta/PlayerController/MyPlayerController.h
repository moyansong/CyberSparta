// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class AMyHUD;
class AMyCharacter;
class AMyPlayerState;
class AMyGameMode;

UCLASS()
class CYBERSPARTA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

//-----------------------------------------------Functions-------------------------------------------------------------
	// Contoller获得一个Pawn时调用,在Server进行
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION(Client, Reliable)
	void ClientOnPossess(APawn* InPawn);

	UFUNCTION()
	void OnPawnChanged(APawn* NewPawn);

	// 获取Server端游戏开始了多久，TimeOfClientRequest为Client发起请求的时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// TimeOfServerReceivedRequest为Server收到请求的时间
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeOfServerReceivedRequest);

	// 从Server的GameMode获取一些信息，例如游戏时间，比赛状态
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float TimeOfWarmup, float TimeOfMatch, float TimeOfStart, float TimeOfSettlement);

	void HandleMatchHasStarted();
	void HandleMatchHasSettled();
public:
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CheckTimeSync(float DeltaTime);

	// 只负责人物进入游戏时的Widget设置，人物重生时的Widget设置不在这里
	void InitializeInProgressWidget();

	void CheckPing(float DeltaTime);
	void HighPingWarning();
	void StopHighPingWarning();
//-----------------------------------------------Set && Get-------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	AMyHUD* GetMyHUD();

	void SetHUDTime();
	void SetHUDHealth();
	void SetHUDShield();
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo();
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDSettlementCountdown(float CountdownTime);

	virtual float GetServerTime();

	// GameMode调用，在Server发生
	void OnMatchStateSet(FName State);

	void OnMatchStateChanged();
//--------------------------------------------Parameters-------------------------------------------------------------
private:
	UPROPERTY()
	AMyCharacter* MyCharacter;
	AMyHUD* MyHUD;

	UPROPERTY()
	AMyPlayerState* MyPlayerState;

	UPROPERTY()
	AMyGameMode* MyGameMode;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float SettlementTime = 0.f;
	float LevelStartingTime = 0.f;
	uint32 CountdownInt = 0;

	// Client和Server的延迟
	float ClientServerDelta = 0.f;

	// 每过这么长时间就确定一次ClientServer时间差
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 10.f;

	float TimeSyncRunningTime = 0.f;

    float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f; // Ping动画已经播放的时间

	UPROPERTY(EditAnywhere, Category = Net)
	float HighPingDuration = 5.f;	 // Ping动画显示的时间

	UPROPERTY(EditAnywhere, Category = Net)
	float CheckPingFrequency = 20.f; // 每过20s检查ping是不是过高, 过高就显示ping动画

	UPROPERTY(EditAnywhere, Category = Net)
	float HighPingThreshold = 50.f;  // ping高过这个数就算高
};
