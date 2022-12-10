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
class UKillAnnouncementWidget;

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

//------------------------------------------------Functions--------------------------------------------------------------
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void KillAnnouncementTimerFinished(UKillAnnouncementWidget* WidgetToRemove);
public:
	// 每一帧都会调用
	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable)
	UUserWidget* CreateAndAddWidget(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable)
	void RemoveWidget(UUserWidget* Widget);

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	void CreateInProgressWidget();
	void RemoveInProgressWidget();

	void CreateMetchStartWidget();

	void CreateSettlementWidget();

	void AddKillAnnouncement(FString AttackerName, FString VictimName);
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

	UPROPERTY(EditAnywhere, Category = Game)
	TSubclassOf<UUserWidget> KillAnnouncementWidgetClass;
	UPROPERTY()
	UKillAnnouncementWidget* KillAnnouncementWidget;

	TArray<UKillAnnouncementWidget*> KillAnnouncementWidgets;
private:
	FHUDPackage HUDPackage;

	void DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairsSpread = 15.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float KillAnnouncementDuration = 3.f;
};
