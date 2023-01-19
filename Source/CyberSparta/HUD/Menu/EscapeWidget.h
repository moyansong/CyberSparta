// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeWidget.generated.h"

class UTextBlock;
class UButton;
class UMultiplayerSessionsSubsystem;
class APlayerController;

UCLASS()
class CYBERSPARTA_API UEscapeWidget : public UUserWidget
{
	GENERATED_BODY()

//------------------------------------------------Functions--------------------------------------------------------------
protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bIsSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();
public:
	void MenuSetup();
	void MenuTearDown();

//------------------------------------------------Parameters--------------------------------------------------------------
private:
	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;
	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
