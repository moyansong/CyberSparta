// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2
// 范围伤害用的，只有WorldStatic会Block
#define ECC_RadialDamage ECollisionChannel::ECC_GameTraceChannel3

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

static void Cout(UObject* WorldContext, const FString& Msg, FColor Color = FColor::Red, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}

static void Cout(UObject* WorldContext, float Num, FColor Color = FColor::Red, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Red, NetPrefix + FString::Printf(TEXT("%f"), Num));
	}
}

