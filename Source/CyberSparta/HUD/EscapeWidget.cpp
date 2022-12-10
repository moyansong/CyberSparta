// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeWidget.h"
#include "Gameframework/PlayerController.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerSessionsSubsystem.h"
#include "../Characters/MyCharacter.h"

void UEscapeWidget::MenuSetup()
{
	bIsFocusable = true;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController ? PlayerController : World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UEscapeWidget::ReturnButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UEscapeWidget::OnDestroySession);
		}
	}
}

void UEscapeWidget::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController ? PlayerController : World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UEscapeWidget::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UEscapeWidget::OnDestroySession);
	}
}

bool UEscapeWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	return true;
}

void UEscapeWidget::OnDestroySession(bool bIsSuccessful)
{
	if (!bIsSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode) // 在Server
		{
			GameMode->ReturnToMainMenuHost();
		}
		else // 在Client
		{
			PlayerController = PlayerController ? PlayerController : World->GetFirstPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UEscapeWidget::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);
	//想简单的退出游戏直接用这个，不做数据处理
	/*if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}*/
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			AMyCharacter* MyCharacter = Cast<AMyCharacter>(FirstPlayerController->GetPawn());
			if (MyCharacter)
			{
				MyCharacter->LeaveGame();
				MyCharacter->OnLeftGame.AddDynamic(this, &UEscapeWidget::OnPlayerLeftGame);
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UEscapeWidget::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

