// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Net/UnrealNetwork.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Buffs/Buff.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UBuffComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBuffComponent, Buffs);
}

void UBuffComponent::AddBuff(ABuff* Buff)
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->HasAuthority())
	{
		bool bIsOwned = false;
		for (auto MyBuff : Buffs)
		{
			if (MyBuff->GetClass() == Buff->GetClass())
			{
				MyBuff->SetCurrBuffDuration(0.f);
				Buff->SetBuffState(EBuffState::EBS_Destroyed);
				bIsOwned = true;
				break;
			}
		}
		if (!bIsOwned)
		{
			Buffs.AddUnique(Buff);
			Buff->SetBuffState(EBuffState::EBS_Effective);
			Buff->SetOwner(GetOwner());
			Buff->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else
	{
		ServerAddBuff(Buff);
	}
}

void UBuffComponent::ServerAddBuff_Implementation(ABuff* Buff)
{
	AddBuff(Buff);
}

void UBuffComponent::RemoveBuff(ABuff* Buff)
{
	if (GetOwner()->HasAuthority())
	{
		Buffs.Remove(Buff);
	}
}


