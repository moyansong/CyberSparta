// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class CYBERSPARTA_API UGameStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
