// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class CYBERSPARTA_API UAttributeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar; 

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeadShotsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
};
