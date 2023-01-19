// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastingGameMode.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "../../CyberSparta.h"
#include "../../Characters/MyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "../GameStates/MyGameState.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../../Weapons/SpecialWeapons/BlastingBomb.h"
#include "../../Actors/BlastingZone.h"

void ABlastingGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	// 爆破模式不重生，后续加入可以观看队友的视角
}

void ABlastingGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// 生成BlastingZone
	if (BlastingZoneClass)
	{
		FActorSpawnParameters ZoneSpawnParams;

		BlastingZone = GetWorld()->SpawnActor<ABlastingZone>(
			BlastingZoneClass,
			BlastingZoneCenter,
			FRotator::ZeroRotator,
			ZoneSpawnParams
		);
		if (BlastingZone)
		{
			DrawDebugSphere(
				GetWorld(),
				BlastingZoneCenter,
				BlastingZoneRadius,
				120,
				FColor::Red,
				false,
				MatchTime
			); 
		}
	}

	DistributeBlastingBomb();
}

void ABlastingGameMode::HandleMatchHasSettled()
{
	
}

void ABlastingGameMode::DistributeBlastingBomb()
{
	// 挑选一个红队成员，让他有个炸弹
	if (!MyGameState) MyGameState = GetGameState<AMyGameState>();
	if (MyGameState)
	{
		APlayerState* ChosenPlayer = MyGameState->RedTeam[FMath::RandRange(0, MyGameState->RedTeam.Num() - 1)];
		if (ChosenPlayer)
		{
			AMyCharacter* ChosenCharacter = Cast<AMyCharacter>(ChosenPlayer->GetPawn());
			if (ChosenCharacter && ChosenCharacter->GetCombatComponent())
			{
				FActorSpawnParameters BombSpawnParams;

				BlastingBomb = GetWorld()->SpawnActor<ABlastingBomb>(
					BlastingBombClass,
					FVector(0.f, 0.f, 100.f),
					FRotator::ZeroRotator,
					BombSpawnParams
					);
				if (BlastingBomb)
				{
					ChosenCharacter->GetCombatComponent()->AddWeapon(BlastingBomb);
					BlastingBomb->Initialize(BlastingZoneCenter, BlastingZoneRadius);
				}
			}
		}
	}
}
