// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ThrowGrenade.h"
#include "../../Characters/MyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "../../Weapons/RangedWeapons/Grenade.h"

void UAnimNotifyState_ThrowGrenade::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(MeshComp->GetOwner());
	if (MyCharacter && MyCharacter->HasAuthority())
	{
		if (AGrenade* Grenade = Cast<AGrenade>(MyCharacter->GetEquippedWeapon()))
		{
			Grenade->SpawnGrenade();
		}
	}
}

void UAnimNotifyState_ThrowGrenade::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UAnimNotifyState_ThrowGrenade::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(MeshComp->GetOwner());
	if (MyCharacter && MyCharacter->HasAuthority() && MyCharacter->GetCombatComponent())
	{
		AGrenade* Grenade = Cast<AGrenade>(MyCharacter->GetEquippedWeapon());
		if (Grenade && Grenade->GetAmmo() == 0)
		{
			MyCharacter->GetCombatComponent()->RemoveWeapon(Grenade);
			MyCharacter->GetCombatComponent()->EquipWeapon(0);
		}
	}
}
