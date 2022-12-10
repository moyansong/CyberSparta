// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillAnnouncementWidget.generated.h"

class UTextBlock;
class UHorizontalBox;

UCLASS()
class CYBERSPARTA_API UKillAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
//------------------------------------------------Functions--------------------------------------------------------------
	void SetKillAnnouncementText(FString AttackerName, FString VictimName);
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;
};
