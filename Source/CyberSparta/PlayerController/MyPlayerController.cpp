// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "../HUD/MyHUD.h"
#include "../HUD/AttributeWidget.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MyHUD = Cast<AMyHUD>(GetHUD());
}

void AMyPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MyHUD = MyHUD ? MyHUD : Cast<AMyHUD>(GetHUD());
	if (MyHUD && MyHUD->AttributeWidget && MyHUD->AttributeWidget->HealthBar && MyHUD->AttributeWidget->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MyHUD->AttributeWidget->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MyHUD->AttributeWidget->HealthText->SetText(FText::FromString(HealthText));
	}
}