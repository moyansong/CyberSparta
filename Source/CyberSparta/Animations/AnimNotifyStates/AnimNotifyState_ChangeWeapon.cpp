// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ChangeWeapon.h"
#include "../../Characters/MyCharacter.h"
#include "../../Components/CombatComponent.h"

void UAnimNotifyState_ChangeWeapon::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(MeshComp->GetOwner());
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		
	}
}

void UAnimNotifyState_ChangeWeapon::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UAnimNotifyState_ChangeWeapon::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(MeshComp->GetOwner());
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		
	}
}