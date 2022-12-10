// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastingGameMode.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Components/CombatComponent.h"
#include "../GameStates/MyGameState.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../Weapons/BlastingBomb.h"

void ABlastingGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	// 爆破模式不重生，后续加入可以观看队友的视角
}

void ABlastingGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// 挑选一个红队成员，让他有个炸弹
	MyGameState = MyGameState ? MyGameState : GetGameState<AMyGameState>();
	if (MyGameState)
	{
		APlayerState* ChosenPlayer = MyGameState->RedTeam[FMath::RandRange(0, MyGameState->RedTeam.Num() - 1)];
		if (ChosenPlayer)
		{
			AMyCharacter* ChosenCharacter = Cast<AMyCharacter>(ChosenPlayer->GetPawn());
			if (ChosenCharacter && ChosenCharacter->GetCombatComponent())
			{
				FActorSpawnParameters SpawnParams;

				ABlastingBomb* BlastingBomb = GetWorld()->SpawnActor<ABlastingBomb>(
					BlastingBombClass,
					FVector(0.f, 0.f, 100.f),
					FRotator::ZeroRotator,
					SpawnParams
				);
				if (BlastingBomb)
				{
					ChosenCharacter->GetCombatComponent()->AddWeapon(BlastingBomb);
				}
			}
		}
	}
}
