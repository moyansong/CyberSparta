// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

/**
 * 显示在人物头顶上的Widget
 */
UCLASS()
class CYBERSPARTA_API UOverheadWidget : public UUserWidget
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
