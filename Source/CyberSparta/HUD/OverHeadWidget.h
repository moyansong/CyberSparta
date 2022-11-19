// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class CYBERSPARTA_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

	void SetDisplayText(FString DisplayTextString);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn, const FString& RoleType = "Local");

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

};
