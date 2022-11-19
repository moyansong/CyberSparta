// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController/MyPlayerController.h"
#include "../GameMode/MyGameMode.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUD();
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAttributeComponent, Health);
}

void UAttributeComponent::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	// 这个函数只会在Server调用
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(InstigatorController, DamageCauser, GetOwner(), this, Damage, Health);
	UpdateHUD();

	if (!IsAlive() && MyCharacter)
	{
		MyCharacter->Elim();
		AMyGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
		AMyPlayerController* VictimController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		AMyPlayerController* AttackController = Cast<AMyPlayerController>(InstigatorController);
		if (MyGameMode && VictimController && AttackController)
		{
			MyGameMode->PlayerEliminated(MyCharacter, VictimController, AttackController);
		}
	}
}

void UAttributeComponent::OnRep_Health()
{
	UpdateHUD();
}
void UAttributeComponent::UpdateHUD()
{
	MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
	if (MyController)
	{
		MyController->SetHUDHealth(Health, MaxHealth);
	}
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

