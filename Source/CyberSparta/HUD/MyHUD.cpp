// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUD.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Player/AttributeWidget.h"
#include "Player/WeaponWidget.h"
#include "GameState/GameStateWidget.h"
#include "GameState/AnnouncementWidget.h"
#include "GameState/SettlementWidget.h"
#include "GameState/KillAnnouncementWidget.h"
#include "../CyberSparta.h"
#include "../Game/PlayerControllers/MyPlayerController.h"

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	MyController = Cast<AMyPlayerController>(GetOwningPlayerController());
}

void AMyHUD::CreateInProgressWidget()
{
	WeaponWidget = Cast<UWeaponWidget>(CreateAndAddWidget(WeaponWidgetClass));
	AttributeWidget = Cast<UAttributeWidget>(CreateAndAddWidget(AttributeWidgetClass));
	GameStateWidget = Cast<UGameStateWidget>(CreateAndAddWidget(GameStateWidgetClass));
}

void AMyHUD::RemoveInProgressWidget()
{
	if (WeaponWidget) WeaponWidget->RemoveFromParent();
	if (AttributeWidget) AttributeWidget->RemoveFromParent();
	if (GameStateWidget) GameStateWidget->RemoveFromParent();
}

void AMyHUD::CreateMetchStartWidget()
{
	AnnouncementWidget = Cast<UAnnouncementWidget>(CreateAndAddWidget(AnnouncementWidgetClass));
}

void AMyHUD::CreateSettlementWidget()
{
	SettlementWidget = Cast<USettlementWidget>(CreateAndAddWidget(SettlementWidgetClass));
}

void AMyHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = MaxCrosshairsSpread * HUDPackage.CrosshairSpread;

		DrawCrosshairs(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairsColor);
		DrawCrosshairs(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		DrawCrosshairs(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		DrawCrosshairs(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairsColor);
		DrawCrosshairs(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairsColor);
	}
}

void AMyHUD::DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor)
{
	if (!Texture) return;

	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor
	);
}

UUserWidget* AMyHUD::CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!MyController) MyController = Cast<AMyPlayerController>(GetOwningPlayerController());
	if (MyController && WidgetClass)
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(MyController, WidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
			return Widget;
		}
	}
	return nullptr;
}

void AMyHUD::RemoveWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
	}
}

void AMyHUD::AddKillAnnouncement(FString AttackerName, FString VictimName)
{
	KillAnnouncementWidget = Cast<UKillAnnouncementWidget>(CreateAndAddWidget(KillAnnouncementWidgetClass));
	if (KillAnnouncementWidget)
	{
		KillAnnouncementWidget->SetKillAnnouncementText(AttackerName, VictimName);
		
		// 将之前所有KillAnnouncementWidget下移
		// Fix me: KillAnnouncementWidget过多时移除部分KillAnnouncementWidget
		for (auto Widget : KillAnnouncementWidgets)
		{
			if (Widget && Widget->AnnouncementBox)
			{
				UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Widget->AnnouncementBox);
				if (CanvasSlot)
				{
					FVector2D Position = CanvasSlot->GetPosition();
					FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y + CanvasSlot->GetSize().Y);
					CanvasSlot->SetPosition(NewPosition);
				}
			}
		}
		
		KillAnnouncementWidgets.Add(KillAnnouncementWidget);

		FTimerHandle KillAnnouncementTimer;
		FTimerDelegate KillAnnouncementDelegate;
		KillAnnouncementDelegate.BindUFunction(this, FName("KillAnnouncementTimerFinished"), KillAnnouncementWidget);
		GetWorldTimerManager().SetTimer(
			KillAnnouncementTimer,
			KillAnnouncementDelegate,
			KillAnnouncementDuration,
			false
		);
	}
}

void AMyHUD::KillAnnouncementTimerFinished(UKillAnnouncementWidget* WidgetToRemove)
{
	if (WidgetToRemove)
	{
		WidgetToRemove->RemoveFromParent();
	}
}
