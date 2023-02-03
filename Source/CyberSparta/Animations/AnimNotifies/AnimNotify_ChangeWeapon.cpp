// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ChangeWeapon.h"
#include "../../Characters/MyCharacter.h"

void UAnimNotify_ChangeWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(MeshComp->GetOuter()))
	{
		MyCharacter->ChangeWeapon(Value);
	}
}
