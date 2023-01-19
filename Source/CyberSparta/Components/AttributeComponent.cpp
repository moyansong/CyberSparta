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
#include "../Game/PlayerControllers/MyPlayerController.h"
#include "../Game/GameModes/MyGameMode.h"
#include "../Game/PlayerStates/MyPlayerState.h"
#include "../HUD/MyHUD.h"
#include "../HUD/Player/AttributeWidget.h"
#include "../GameFramework/HeadShotDamageType.h"
#include "../GameFramework/TrunkShotDamageType.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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

void UAttributeComponent::DecreaseHealth(float HealthDecrement)
{
	SetHealth(Health - HealthDecrement);
}

void UAttributeComponent::OnRep_Health(float OldHealth)
{
	HealthChangedDelegate.Broadcast(GetOwner(), OldHealth, Health);
	SetHUDHealth();
	if (!IsAlive() && MyCharacter)
	{
		Eliminate(nullptr, nullptr, nullptr);
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

void UAttributeComponent::DecreaseShield(float ShieldDecrement)
{
	SetShield(Shield - ShieldDecrement);
}

void UAttributeComponent::OnRep_Shield(float OldShield)
{
	SetHUDShield();
}

float UAttributeComponent::CalculateShiledReceivedDamage(float Damage)
{
	return FMath::Min(Shield, Damage * ShieldDamageReduction);
}

FDamage UAttributeComponent::CalculateReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (!IsAlive() || (IsTeammate(InstigatorController) && !bUseTeammateDamage))
	{
		return { false, 0.f, 0.f, 0.f};
	}
	if (DamageType && DamageType->GetClass() == UHeadShotDamageType::StaticClass())
	{
		return { true, Damage, Damage, 0.f };
	}
	if (DamageType && DamageType->GetClass() == UTrunkShotDamageType::StaticClass())
	{
		float DamageToShiled = CalculateShiledReceivedDamage(Damage);
		return { false, Damage, Damage - DamageToShiled, DamageToShiled };
	}
	return { false, Damage, Damage, 0.f };
}

void UAttributeComponent::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	FDamage ReceivedDamage = CalculateReceivedDamage(DamageActor, Damage, DamageType, InstigatorController, DamageCauser);
	if (ReceivedDamage.TotalDamage == 0.f) return;
	
	const float NewHealth = Health - ReceivedDamage.DamageToHealth;
	HealthChangedDelegate.Broadcast(GetOwner(), Health, NewHealth);
	SetHealth(NewHealth);

	const float NewShield = Shield - ReceivedDamage.DamageToShield;
	SetShield(NewShield);

	if (!IsAlive() && MyCharacter)
	{
		Eliminate(DamageActor, InstigatorController, DamageCauser, ReceivedDamage.bHeadShot);
	}
}

bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::Eliminate(AActor* DamageActor, AController* InstigatorController, AActor* DamageCauser, bool bHeadShot)
{
	if (!MyCharacter || IsAlive()) return;
	
	SetEliminatedCollision();
	MyCharacter->Eliminate();
	MyCharacter->SetDisableGameplay(true); 

	if (MyCharacter->HasAuthority())
	{
		if (!MyGameMode) MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
		AMyPlayerController* AttackerController = Cast<AMyPlayerController>(InstigatorController);
		AMyPlayerController* VictimController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
		if (MyGameMode && AttackerController && VictimController)
		{
			MyGameMode->PlayerEliminated(MyCharacter, AttackerController, VictimController, bHeadShot);
		}

		MyCharacter->GetWorldTimerManager().SetTimer(
			EliminateTimer,
			this,
			&UAttributeComponent::EliminateTimerFinished,
			EliminateDelay
		);

		MyCharacter->KillReward();
	}
}

void UAttributeComponent::EliminateTimerFinished()
{
	if (!MyGameMode) MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
	if (MyGameMode && MyCharacter && MyController)
	{
		MyGameMode->RequestRespawn(MyCharacter, MyController);
	}
}

void UAttributeComponent::SetEliminatedCollision()
{
	if (IsAlive() || !MyCharacter || !MyCharacter->GetCapsuleComponent() || !MyCharacter->GetMesh()) return;

	//MyCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MyCharacter->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MyCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	
	// 注意ServerSideRewind会在每个端生成子弹，子弹是和Mesh碰撞的
	// 如果发射子弹的本地端中的子弹先与这个人碰撞，Elim通过Health复制在所有客户端上执行
	// 如果执行的很快，在其他端的子弹与其碰撞前将他的Mesh碰撞取消，其他客户端的子弹可能会穿过他打到别人，虽然不会造成伤害，但会播放动画
	// 发生这种事的概率很小，真实网络环境中，变量复制需要时间，而且子弹飞行速度很快
	// 所以这里还是让Mesh可以Block子弹
	//MyCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MyCharacter->GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MyCharacter->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
}

bool UAttributeComponent::IsTeammate(AController* OtherPlayerController)
{
	if (MyController && !MyPlayerState)
	{
		MyPlayerState = MyController->GetPlayerState<AMyPlayerState>();
	}
	return OtherPlayerController && MyPlayerState && MyPlayerState->IsTeammate(OtherPlayerController->GetPlayerState<AMyPlayerState>());
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

void UAttributeComponent::SetHUDAttribute()
{
	SetHUDHealth();
	SetHUDShield();
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

	if (AttributeWidget->DefeatsText)
	{
		FString DefeatsString = FString::Printf(TEXT("%d"), Defeats);
		AttributeWidget->DefeatsText->SetText(FText::FromString(DefeatsString));
	}
}

void UAttributeComponent::SetHUDHeadShots(int32 HeadShots)
{
	if (!IsHUDVaild()) return;

	if (AttributeWidget->HeadShotsText)
	{
		FString HeadShotsString = FString::Printf(TEXT("%d"), HeadShots);
		AttributeWidget->HeadShotsText->SetText(FText::FromString(HeadShotsString)); 
	}
}


