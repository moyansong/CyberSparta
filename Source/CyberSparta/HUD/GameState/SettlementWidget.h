// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettlementWidget.generated.h"

class UTextBlock;
class UButton;
class UMultiplayerSessionsSubsystem;
class APlayerController;

UCLASS()
class CYBERSPARTA_API USettlementWidget : public UUserWidget
{
	GENERATED_BODY()

//------------------------------------------------Parameters--------------------------------------------------------------
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SettlementTimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WinnerText;
};
