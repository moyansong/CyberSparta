// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponWidget.generated.h"

class UTextBlock;

UCLASS()
class CYBERSPARTA_API UWeaponWidget : public UUserWidget
{
	GENERATED_BODY()

public:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;
};
