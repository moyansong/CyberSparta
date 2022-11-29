// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
拾取，开关门，上车等等互动都用这个接口
 */
class CYBERSPARTA_API IInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION()
	virtual void InteractStart(AActor* InteractActor) = 0;

	UFUNCTION()
	virtual void InteractStop(AActor* InteractActor) = 0;

	// 设置互动时的效果，例如展现Widget，产生粒子等，bVisibility为真表示显示效果，反之不显示
	UFUNCTION()
	virtual void SetInteractEffectVisibility(bool bVisibility) = 0;
};
