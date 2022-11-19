// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

class UTexture2D;
class UUserWidget;
class UAttributeWidget;

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

/**
 * 
 */
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

//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Attribute)
	TSubclassOf<UUserWidget> AttributeWidgetClass;

	UAttributeWidget* AttributeWidget;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairsSpread = 15.f;
};
