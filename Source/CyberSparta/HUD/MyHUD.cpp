// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHUD.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "../CyberSparta.h"
#include "../PlayerController/MyPlayerController.h"
#include "../HUD/AttributeWidget.h"
#include "../HUD/WeaponWidget.h"
#include "../HUD/GameStateWidget.h"
#include "../HUD/AnnouncementWidget.h"
#include "../HUD/SettlementWidget.h"

void AMyHUD::BeginPlay()
{
	Super::BeginPlay();

	MyController = Cast<AMyPlayerController>(GetOwningPlayerController());
}

void AMyHUD::CreateInProgressWidget()
{
	AttributeWidget = Cast<UAttributeWidget>(CreateAndAddWidget(AttributeWidgetClass));
	WeaponWidget = Cast<UWeaponWidget>(CreateAndAddWidget(WeaponWidgetClass));
	GameStateWidget = Cast<UGameStateWidget>(CreateAndAddWidget(GameStateWidgetClass));
}

void AMyHUD::RemoveInProgressWidget()
{
	if (AttributeWidget) AttributeWidget->RemoveFromParent();
	if (WeaponWidget) WeaponWidget->RemoveFromParent();
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
	MyController = MyController ? MyController : Cast<AMyPlayerController>(GetOwningPlayerController());
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

