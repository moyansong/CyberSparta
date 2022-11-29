// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

class AMyPlayerController;
class UTexture2D;
class UUserWidget;
class UAttributeWidget;
class UWeaponWidget;
class UGameStateWidget;
class UAnnouncementWidget;
class USettlementWidget;


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

UCLASS()
class CYBERSPARTA_API AMyHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	// 每一帧都会调用
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable)
	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable)
	void RemoveWidget(UUserWidget* Widget);

//------------------------------------------------Functions--------------------------------------------------------------
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	void CreateInProgressWidget();
	void RemoveInProgressWidget();

	void CreateMetchStartWidget();

	void CreateSettlementWidget();
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY()
	AMyPlayerController* MyController;
	
	UPROPERTY(EditAnywhere, Category = Attribute)
	TSubclassOf<UUserWidget> AttributeWidgetClass;
	UPROPERTY()
	UAttributeWidget* AttributeWidget;

	UPROPERTY(EditAnywhere, Category = Weapon)
	TSubclassOf<UUserWidget> WeaponWidgetClass;
	UPROPERTY()
	UWeaponWidget* WeaponWidget;

	UPROPERTY(EditAnywhere, Category = Game)
	TSubclassOf<UUserWidget> GameStateWidgetClass;
	UPROPERTY()
	UGameStateWidget* GameStateWidget;

	UPROPERTY(EditAnywhere, Category = Game)
	TSubclassOf<UUserWidget> AnnouncementWidgetClass;
	UPROPERTY()
	UAnnouncementWidget* AnnouncementWidget;

	UPROPERTY(EditAnywhere, Category = Game)
	TSubclassOf<UUserWidget> SettlementWidgetClass;
	UPROPERTY()
	USettlementWidget* SettlementWidget;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairsSpread = 15.f;
};
