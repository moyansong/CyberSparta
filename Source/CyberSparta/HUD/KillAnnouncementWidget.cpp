// Fill out your copyright notice in the Description page of Project Settings.


#include "KillAnnouncementWidget.h"
#include "Components/TextBlock.h"

void UKillAnnouncementWidget::SetKillAnnouncementText(FString AttackerName, FString VictimName)
{
	FString KillAnnouncemtnString = FString::Printf(TEXT("%s killed %s"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(KillAnnouncemtnString));
	}
}
