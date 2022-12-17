// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "../CyberSparta.h"
#include "../GameMode/MyGameMode.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../Components/AttributeComponent.h"
#include "../Components/CombatComponent.h"
#include "../Characters/MyCharacter.h"
#include "../GameStates/MyGameState.h"
#include "../Types/Announcement.h"
#include "../HUD/MyHUD.h"
#include "../HUD/AttributeWidget.h"
#include "../HUD/GameStateWidget.h"
#include "../HUD/AnnouncementWidget.h"
#include "../HUD/SettlementWidget.h"
#include "../HUD/EscapeWidget.h"


void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MyHUD = Cast<AMyHUD>(GetHUD());
	MyPlayerState = GetPlayerState<AMyPlayerState>();

	ServerCheckMatchState();

	// OnNewPawn.AddUObject(this, &AMyPlayerController::OnPawnChanged);
}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	CheckPing(DeltaTime);
}

void AMyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerController, MatchState);
	DOREPLIFETIME(AMyPlayerController, bIsTeamMatch);
}

void AMyPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		MyPlayerState = MyPlayerState ? MyPlayerState : GetPlayerState<AMyPlayerState>();
		if (MyPlayerState)
		{
			if (MyPlayerState->GetPingInMilliseconds() > HighPingThreshold)
			{
				HighPingWarning();
				ServerReportPingStatus(MyPlayerState->GetPingInMilliseconds());
				PingAnimationRunningTime = 0.f;
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bIsHighPingAnimationPlaying =
		MyHUD && MyHUD->GameStateWidget &&
		MyHUD->GameStateWidget->HighPingAnimation &&
		MyHUD->GameStateWidget->IsAnimationPlaying(MyHUD->GameStateWidget->HighPingAnimation);
	if (bIsHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AMyPlayerController::ServerReportPingStatus_Implementation(float Ping)
{
	// Ping太高就取消一些设置，比如不广播人物的动作特效之类的，由于广播是由Server做，所以在Server上设置就可以
	HigPingDelegate.Broadcast(Ping);
}

void AMyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent) return;
	
	InputComponent->BindAction("Escape", IE_Pressed, this, &AMyPlayerController::ShowEscapeWidget);
}

void AMyPlayerController::ClientOnPossess_Implementation(APawn* InPawn)
{
	
}

float AMyPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + (HasAuthority() ? 0.f : ClientServerDelta);
}

void AMyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float CurrServerTime = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, CurrServerTime);
}

void AMyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeOfServerReceivedRequest)
{
	// 两次RPC消耗的时间
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	float CurrServerTime = TimeOfServerReceivedRequest + SingleTripTime;
	ClientServerDelta = CurrServerTime - GetWorld()->GetTimeSeconds();
}

void AMyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Settlement) TimeLeft = WarmupTime + MatchTime + SettlementTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		MyGameMode = MyGameMode ? MyGameMode : Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
		SecondsLeft = FMath::CeilToInt(MyGameMode->GetCountdownTime() + LevelStartingTime);
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::Settlement)
		{
			SetHUDSettlementCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void AMyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AMyPlayerController::ServerCheckMatchState_Implementation()
{
	MyGameMode = MyGameMode ? MyGameMode : Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (MyGameMode)
	{
		WarmupTime = MyGameMode->WarmupTime;
		MatchTime = MyGameMode->MatchTime;
		SettlementTime = MyGameMode->SettlementTime;
		LevelStartingTime = MyGameMode->LevelStartingTime;
		MatchState = MyGameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, SettlementTime);
	}
}

void AMyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float TimeOfWarmup, float TimeOfMatch, float TimeOfStart, float TimeOfSettlement)
{
	WarmupTime = TimeOfWarmup;
	MatchTime = TimeOfMatch;
	SettlementTime = TimeOfSettlement;
	LevelStartingTime = TimeOfStart;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (MyHUD && MatchState == MatchState::WaitingToStart)
	{
		MyHUD->CreateMetchStartWidget();
	}
}

void AMyPlayerController::InitializeInProgressWidget()
{
	SetHUDHealth();
	SetHUDShield();
	SetHUDWeapon();
	InitializeTeamScore();
	MyPlayerState = MyPlayerState ? MyPlayerState :GetPlayerState<AMyPlayerState>();
	if (MyPlayerState)
	{
		SetHUDScore(MyPlayerState->GetScore());
		SetHUDDefeats(MyPlayerState->GetDefeats());
	}
}

AMyHUD* AMyPlayerController::GetMyHUD()
{
	return MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
}

void AMyPlayerController::OnPawnChanged(APawn* NewPawn)
{
	MyCharacter = Cast<AMyCharacter>(NewPawn);
	if (MyCharacter)
	{
		MyCharacter->UpdateHUD(GetMyHUD());
	}
}

void AMyPlayerController::OnMatchStateSet(FName State, bool bTeamMatch)
{
	MatchState = State;
	bIsTeamMatch = bTeamMatch;
	OnMatchStateChanged();
}

void AMyPlayerController::OnRep_MatchState()
{
	OnMatchStateChanged();
}

void AMyPlayerController::OnRep_bIsTeamMatch()
{
	InitializeTeamScore();
}

void AMyPlayerController::OnMatchStateChanged()
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD()); 

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Settlement)
	{
		HandleMatchHasSettled();
	}
}

void AMyPlayerController::HandleMatchHasStarted()
{
	if (!MyHUD) return;

	MyHUD->RemoveWidget(MyHUD->AnnouncementWidget);
	MyHUD->CreateInProgressWidget();
	InitializeInProgressWidget();
}

void AMyPlayerController::HandleMatchHasSettled()
{
	if (!MyHUD) return;

	MyHUD->RemoveInProgressWidget();
	MyHUD->CreateSettlementWidget();
	SetHUDWinner();

	MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter)
	{
		MyCharacter->SetDisableGameplay(true);
	}
}

void AMyPlayerController::SetHUDHealth()
{
	MyCharacter = Cast<AMyCharacter>(GetPawn());// 删了就不行，这里要获取本地的Pawn
	if (MyCharacter && MyCharacter->GetAttributeComponent())
	{
		MyCharacter->GetAttributeComponent()->SetHUDHealth(); 
	}
}

void AMyPlayerController::SetHUDShield()
{
	MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->GetAttributeComponent())
	{
		MyCharacter->GetAttributeComponent()->SetHUDShield();
	}
}

void AMyPlayerController::SetHUDScore(float Score)
{
	MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->GetAttributeComponent())
	{
		MyCharacter->GetAttributeComponent()->SetHUDScore(Score);
	}
}

void AMyPlayerController::SetHUDDefeats(int32 Defeats)
{
	MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->GetAttributeComponent())
	{
		MyCharacter->GetAttributeComponent()->SetHUDDefeats(Defeats);
	}
}

void AMyPlayerController::InitializeTeamScore()
{
	if (bIsTeamMatch)
	{
		SetHUDBlueTeamScore(0);
		SetHUDRedTeamScore(0);
	}
	else
	{
		HideTeamScore();
	}
}

void AMyPlayerController::HideTeamScore()
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->BlueTeamScoreText)
	{
		MyHUD->GameStateWidget->BlueTeamScoreText->SetText(FText());
	}
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->RedTeamScoreText)
	{
		MyHUD->GameStateWidget->RedTeamScoreText->SetText(FText());
	}
}

void AMyPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->BlueTeamScoreText)
	{
		FString ScoreString = FString::Printf(TEXT("%d"), BlueScore);
		MyHUD->GameStateWidget->BlueTeamScoreText->SetText(FText::FromString(ScoreString));
	}
}

void AMyPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->RedTeamScoreText)
	{
		FString ScoreString = FString::Printf(TEXT("%d"), RedScore);
		MyHUD->GameStateWidget->RedTeamScoreText->SetText(FText::FromString(ScoreString));
	}
}

void AMyPlayerController::SetHUDWeapon()
{
	MyCharacter = Cast<AMyCharacter>(GetPawn());
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		MyCharacter->GetCombatComponent()->SetHUDWeapon();
	}
}

void AMyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->MatchCountdownText)
	{
		if (CountdownTime < 0.f)
		{
			MyHUD->GameStateWidget->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MyHUD->GameStateWidget->MatchCountdownText->SetText(FText::FromString(CountdownString));
	}
}

void AMyPlayerController::SetHUDWinner()
{
	AMyGameState* MyGameState = Cast<AMyGameState>(UGameplayStatics::GetGameState(this));
	if (MyGameState && MyPlayerState && MyHUD->SettlementWidget && MyHUD->SettlementWidget->WinnerText)
	{
		TArray<AMyPlayerState*> TopPlayers = MyGameState->TopScoringPlayers;
		FString WinnerString;
		if (bIsTeamMatch)
		{
			if (MyGameState->BlueTeamScore > MyGameState->RedTeamScore)
			{
				WinnerString = Announcement::BlueTeamWin;
			}
			else if (MyGameState->BlueTeamScore < MyGameState->RedTeamScore)
			{
				WinnerString = Announcement::RedTeamWin;
			}
			else
			{
				WinnerString = Announcement::TeamsTiedForWin;
			}
		}
		else if (TopPlayers.IsEmpty())
		{
			WinnerString = Announcement::NoWinner;
		}
		else if (TopPlayers.Num() == 1 && TopPlayers[0] == MyPlayerState)
		{
			WinnerString = Announcement::YouAreTheWinner;
		}
		else if (TopPlayers.Num() == 1)
		{
			// \n是换行
			WinnerString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
		}
		else if (TopPlayers.Num() > 1)
		{
			WinnerString = Announcement::PlayersTiedForWin;
			for (auto TopPlayer : TopPlayers)
			{
				WinnerString.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
			}
		}
		MyHUD->SettlementWidget->WinnerText->SetText(FText::FromString(WinnerString));
	}
}

void AMyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->AnnouncementWidget && MyHUD->AnnouncementWidget->WarmupTimeText)
	{
		if (CountdownTime < 0.f)
		{
			MyHUD->AnnouncementWidget->WarmupTimeText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MyHUD->AnnouncementWidget->WarmupTimeText->SetText(FText::FromString(CountdownString));
	}
}

void AMyPlayerController::SetHUDSettlementCountdown(float CountdownTime)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->SettlementWidget && MyHUD->SettlementWidget->SettlementTimeText)
	{
		if (CountdownTime < 0.f)
		{
			MyHUD->SettlementWidget->SettlementTimeText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MyHUD->SettlementWidget->SettlementTimeText->SetText(FText::FromString(CountdownString));
	}
}

void AMyPlayerController::HighPingWarning()
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->HighPingImage && MyHUD->GameStateWidget->HighPingAnimation)
	{
		MyHUD->GameStateWidget->HighPingImage->SetOpacity(1.f); 
		MyHUD->GameStateWidget->PlayAnimation(
			MyHUD->GameStateWidget->HighPingAnimation,
			0.f,
			5
		);
	}
}

void AMyPlayerController::StopHighPingWarning()
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->GameStateWidget && MyHUD->GameStateWidget->HighPingImage && MyHUD->GameStateWidget->HighPingAnimation)
	{
		MyHUD->GameStateWidget->HighPingImage->SetOpacity(0.f);
		if (MyHUD->GameStateWidget->IsAnimationPlaying(MyHUD->GameStateWidget->HighPingAnimation))
		{
			MyHUD->GameStateWidget->StopAnimation(MyHUD->GameStateWidget->HighPingAnimation);
		}
	}
}

void AMyPlayerController::ShowEscapeWidget()
{
	if (!EscapeWidgetClass) return;
	if (!EscapeWidget)
	{
		EscapeWidget = CreateWidget<UEscapeWidget>(this, EscapeWidgetClass);
	}
	if (EscapeWidget)
	{
		bEscapeWidgetOpen = !bEscapeWidgetOpen;
		if (bEscapeWidgetOpen)
		{
			EscapeWidget->MenuSetup();
		}
		else
		{
			EscapeWidget->MenuTearDown();
		}
	}
}

void AMyPlayerController::BroadcastKill(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState)
{
	ClientKillAnnouncement(AttackerPlayerState, VictimPlayerState);
}

void AMyPlayerController::ClientKillAnnouncement_Implementation(AMyPlayerState* AttackerPlayerState, AMyPlayerState* VictimPlayerState)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && AttackerPlayerState && VictimPlayerState)
	{
		// 如果是用Steam连接的，PlayerName是Steam名字
		MyHUD->AddKillAnnouncement(AttackerPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName());
	}
}

