// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController/MyPlayerController.h"
#include "../GameMode/MyGameMode.h"
#include "../HUD/MyHUD.h"
#include "../HUD/AttributeWidget.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);

	Health = MaxHealth;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAttributeComponent, Health);
	DOREPLIFETIME(UAttributeComponent, Shield);
}

void UAttributeComponent::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
	SetHUDHealth();
}

void UAttributeComponent::IncreaseHealth(float HealthIncrement)
{
	SetHealth(Health + HealthIncrement);
}

void UAttributeComponent::OnRep_Health(float LastHealth)
{
	SetHUDHealth();
	if (!IsAlive() && MyCharacter)
	{
		Elim(nullptr, nullptr, nullptr);
	}
}

void UAttributeComponent::SetShield(float NewShield)
{
	Shield = FMath::Clamp(NewShield, 0.f, MaxShield);
	SetHUDShield();
}

void UAttributeComponent::IncreaseShield(float ShieldIncrement)
{
	SetShield(Shield + ShieldIncrement);
}

void UAttributeComponent::OnRep_Shield(float LastShield)
{
	SetHUDShield();
}

void UAttributeComponent::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	SetHealth(Health - Damage);
	OnHealthChanged.Broadcast(InstigatorController, DamageCauser, GetOwner(), this, Damage, Health);

	if (!IsAlive() && MyCharacter)
	{
		Elim(DamageActor, InstigatorController, DamageCauser);
	}
}

bool UAttributeComponent::IsHUDVaild()
{
	if (!MyCharacter)
	{
		MyCharacter = Cast<AMyCharacter>(GetOwner());
		if (!MyCharacter) return false;
	}
	if (!MyController)
	{
		MyController = Cast<AMyPlayerController>(MyCharacter->Controller);
		if (!MyController) return false;
	}
	if (!MyHUD)
	{
		MyHUD = MyController->GetMyHUD();
		if (!MyHUD) return false;
	}
	if (!AttributeWidget)
	{
		AttributeWidget = MyHUD->AttributeWidget;
		if (!AttributeWidget) return false;
	}
	return true;
}

void UAttributeComponent::UpdateHUD(AMyHUD* PlayerHUD)
{
	if (PlayerHUD)
	{
		MyHUD = PlayerHUD;
	}
	if (MyHUD)
	{
		AttributeWidget = AttributeWidget ? AttributeWidget : MyHUD->AttributeWidget;
		SetHUDHealth();
	}
}

void UAttributeComponent::SetHUDHealth()
{
	if (!IsHUDVaild()) return;
	
	if (AttributeWidget->HealthBar && AttributeWidget->HealthText)
	{
		const float HealthPercent = Health / MaxHealth; 
		AttributeWidget->HealthBar->SetPercent(HealthPercent);
		FString HealthString = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		AttributeWidget->HealthText->SetText(FText::FromString(HealthString));
	}
}

void UAttributeComponent::SetHUDShield()
{
	if (!IsHUDVaild()) return;

	if (AttributeWidget->ShieldBar && AttributeWidget->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		AttributeWidget->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldString = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		AttributeWidget->ShieldText->SetText(FText::FromString(ShieldString));
	}
}

void UAttributeComponent::SetHUDScore(float Score)
{
	if (!IsHUDVaild()) return;
	
	if (AttributeWidget->ScoreText)
	{
		FString ScoreString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); 
		AttributeWidget->ScoreText->SetText(FText::FromString(ScoreString));
	}
}

void UAttributeComponent::SetHUDDefeats(int32 Defeats)
{
	if (!IsHUDVaild()) return;

	if (AttributeWidget->ScoreText)
	{
		FString DefeatsString = FString::Printf(TEXT("%d"), Defeats);
		AttributeWidget->DefeatsText->SetText(FText::FromString(DefeatsString));
	}
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::Elim(AActor* DamageActor, AController* InstigatorController, AActor* DamageCauser)
{
	if (!MyCharacter || IsAlive()) return;
	
	MyCharacter->SimulateElim();

	MyCharacter->GetCharacterMovement()->DisableMovement();
	MyCharacter->GetCharacterMovement()->StopMovementImmediately();
	MyCharacter->SetDisableGameplay(true);
	MyCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MyCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (MyCharacter->HasAuthority())
	{
		AMyGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
		AMyPlayerController* VictimController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		AMyPlayerController* AttackController = Cast<AMyPlayerController>(InstigatorController);
		if (MyGameMode && VictimController && AttackController)
		{
			MyGameMode->PlayerEliminated(MyCharacter, VictimController, AttackController); 
		}

		MyCharacter->GetWorldTimerManager().SetTimer(
			ElimTimer,
			this,
			&UAttributeComponent::ElimTimerFinished,
			ElimDelay
		);

		MyCharacter->KillReward();
	}
}

void UAttributeComponent::ElimTimerFinished()
{
	AMyGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
	if (MyGameMode && MyCharacter && MyController)
	{
		MyGameMode->RequestRespawn(MyCharacter, MyController);
	}
}


