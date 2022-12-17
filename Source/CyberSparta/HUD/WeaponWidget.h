// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponWidget.generated.h"

class UTextBlock;
class UTexture2D;
class UImage;
class AWeapon;

UCLASS()
class CYBERSPARTA_API UWeaponWidget : public UUserWidget
{
	GENERATED_BODY()

public:
//------------------------------------------------Parameters--------------------------------------------------------------
	void SetHUDWeaponImage(AWeapon* LastWeapon, AWeapon* Weapon);
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(meta = (BindWidget))
	UImage* WeaponImage;
};
