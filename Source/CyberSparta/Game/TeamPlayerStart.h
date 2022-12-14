// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "../Types/Team.h"
#include "TeamPlayerStart.generated.h"

UCLASS()
class CYBERSPARTA_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Paramete)
	ETeam Team;
};
