// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

class AMyCharacter;
class AMyPlayerController;

UCLASS()
class CYBERSPARTA_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
//------------------------------------------------Functions------------------------------------------------------------
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_Score() override;

	void IncreaseScore(float ScoreIncrement);

	UFUNCTION()
	virtual void OnRep_Defeats();

	void IncreaseDefeats(int32 DefeatsIncrement);

	FORCEINLINE int32 GetDefeats() const { return Defeats; }
private:
//------------------------------------------------Parameters--------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;

	// 被淘汰/击杀的次数
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats = 0;

};
