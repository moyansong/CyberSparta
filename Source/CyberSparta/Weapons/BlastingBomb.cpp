// Fill out your copyright notice in the Description page of Project Settings.


#include "BlastingBomb.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController/MyPlayerController.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../Components/CombatComponent.h"
#include "../Components/AttributeComponent.h"
#include "../GameMode/BlastingGameMode.h"

ABlastingBomb::ABlastingBomb()
{
	Ammo = 1;
	MaxAmmo = 1;
	AmmoCostPerFire = 0;
	bReplicates = true;
	bCanReload = false;
	bCanAutomaticFire = false;
	FireDelay = InstallDuration + 0.2f;
	Team = ETeam::ET_RedTeam;
}

void ABlastingBomb::Initialize(FVector CenterOfBlastingZone, float RadiusOfBlastingZone)
{
	BlastingZoneCenter = CenterOfBlastingZone;
	BlastingZoneRadius = RadiusOfBlastingZone;
}

void ABlastingBomb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckOwnerState();
}

void ABlastingBomb::BeginPlay()
{
	Super::BeginPlay();
}

void ABlastingBomb::CheckOwnerState()
{
	bool bStopInstall = MyCharacter &&
						MyCharacter->IsLocallyControlled() &&
						MyCharacter->GetTeam() == ETeam::ET_RedTeam &&
						MyCharacter->GetVelocity().Size() > 0.f;
	if (bStopInstall)
	{
		GetWorldTimerManager().ClearTimer(InstallTimer); 
	}

	bool bStopUninstall = MyCharacter &&
						  MyCharacter->IsLocallyControlled() &&
						  MyCharacter->GetTeam() == ETeam::ET_BlueTeam &&
						  (MyCharacter->GetVelocity().Size() > 0.f ||
						  MyCharacter->GetCombatState() != ECombatState::ECS_Idle);
	if (bStopUninstall)
	{
		GetWorldTimerManager().ClearTimer(UninstallTimer);
	}
}

void ABlastingBomb::Destroyed()
{
	// Fix me:播放爆炸特效

	ABlastingGameMode* BlastingGameMode = GetWorld()->GetAuthGameMode<ABlastingGameMode>();
	if (BlastingGameMode)
	{
		BlastingGameMode->SetMatchState(MatchState::Settlement);
	}
	
	Super::Destroyed();
}

void ABlastingBomb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlastingBomb, bIsInstalled);
	DOREPLIFETIME(ABlastingBomb, BlastingZoneCenter);
	DOREPLIFETIME(ABlastingBomb, BlastingZoneRadius);
}

void ABlastingBomb::BindCallbackOfHealthChanged(bool bShouldBind)
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->IsLocallyControlled() && MyCharacter->GetAttributeComponent())
	{
		if (bShouldBind)
		{
			MyCharacter->GetAttributeComponent()->HealthChangedDelegate.AddDynamic(this, &ABlastingBomb::OnHealthChanged);
		}
		else
		{
			MyCharacter->GetAttributeComponent()->HealthChangedDelegate.RemoveDynamic(this, &ABlastingBomb::OnHealthChanged);
		}
	}
}

void ABlastingBomb::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		if (OtherCharacter->GetTeam() == ETeam::ET_RedTeam && !bIsInstalled)
		{
			OtherCharacter->SetOverlappingActor(this);
		}
		else if (OtherCharacter->GetTeam() == ETeam::ET_BlueTeam && bIsInstalled)
		{
			OtherCharacter->SetOverlappingActor(this);
			
			// BlastingBomb在安装后就没有Owner了
			// 这里设置Owner只是为了让与他Overlap的Pawn所在的客户端能拥有这个Bomb
			// 好后面发起在客户端发起ServerRPC(发起Server调用的客户端必须拥有这个Actor)
			SetOwner(OtherActor);
			BindCallbackOfHealthChanged(true);
		}
	}
}

void ABlastingBomb::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->SetOverlappingActor(nullptr);
		if (OtherCharacter->GetTeam() == ETeam::ET_BlueTeam && bIsInstalled)
		{
			SetOwner(nullptr);
		}
	}
}

void ABlastingBomb::OnIdled()
{
	Super::OnIdled();
	BindCallbackOfHealthChanged(true);
}

void ABlastingBomb::OnRep_Owner()
{
	BindCallbackOfHealthChanged(false);

	Super::OnRep_Owner();

	BindCallbackOfHealthChanged(true);
}

void ABlastingBomb::OnHealthChanged(AActor* DamageActor, float OldHealth, float NewHealth)
{
	if (bIsInstalled)
	{
		GetWorldTimerManager().ClearTimer(UninstallTimer);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(InstallTimer);
	}
}

void ABlastingBomb::InteractStart(AActor* InteractActor)
{
	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		if (InteractCharacter->GetTeam() == ETeam::ET_BlueTeam && bIsInstalled)
		{
			GetWorldTimerManager().SetTimer(
				UninstallTimer,
				this,
				&ABlastingBomb::Uninstall,
				1.f,
				false,
				UninstallDuration
			);
		}
	}
}

void ABlastingBomb::InteractStop(AActor* InteractActor)
{
	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		if (InteractCharacter->GetTeam() == ETeam::ET_RedTeam && !bIsInstalled)
		{
			Pickup(InteractCharacter);
		}
		else if (InteractCharacter->GetTeam() == ETeam::ET_BlueTeam && bIsInstalled)
		{
			GetWorldTimerManager().ClearTimer(UninstallTimer);
		}
	}
}

bool ABlastingBomb::CanFire()
{
	return FVector::Distance(GetActorLocation(), BlastingZoneCenter) <= BlastingZoneRadius;
}

void ABlastingBomb::FireStart(const FVector& HitTarget)
{
	if (MyCharacter && MyCharacter->IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimer(
			InstallTimer,
			this,
			&ABlastingBomb::Install,
			InstallDuration,
			false
		);
	}
}

void ABlastingBomb::FireStop()
{
	GetWorldTimerManager().ClearTimer(InstallTimer);
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		MyCharacter->GetCombatComponent()->SetCombatState(ECombatState::ECS_Idle);
	}
}

void ABlastingBomb::Install()
{
	if (HasAuthority())
	{
		bIsInstalled = true;
		OnInstalled();
		SetInstallLocation();
		SetOwner(nullptr);
		SetSphereCollision(true);

		MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
		if (MyCharacter && MyCharacter->GetCombatComponent())
		{
			MyCharacter->GetCombatComponent()->SetCombatState(ECombatState::ECS_Idle);
			MyCharacter->GetCombatComponent()->RemoveWeapon(this);
		}

		ABlastingGameMode* BlastingGameMode = GetWorld()->GetAuthGameMode<ABlastingGameMode>();
		if (BlastingGameMode)
		{
			SetLifeSpan(FMath::Min(BlastingDelay, BlastingGameMode->GetCountdownTime()));
		}
	}
	else
	{
		ServerInstall();
	}
}

void ABlastingBomb::ServerInstall_Implementation()
{
	Install();
}

void ABlastingBomb::SetInstallLocation()
{
	FVector Start = GetActorLocation() + (MyCharacter ? MyCharacter->GetActorForwardVector() * 50.f : FVector::ZeroVector);
	FVector End = FVector(Start.X, Start.Y, -1000.f);
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_RadialDamage
	);
	if (HitResult.bBlockingHit)
	{
		SetActorLocation(HitResult.ImpactPoint);
		SetActorRotation(FRotator::ZeroRotator);
	}
}

void ABlastingBomb::OnInstalled()
{
	SetInteractText("Unistall");
	if (GetStaticMesh())
	{
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
		GetStaticMesh()->DetachFromComponent(DetachRules);
	}
}

void ABlastingBomb::Uninstall()
{
	if (HasAuthority())
	{
		bIsInstalled = false;
		OnUninstalled();
		
		ABlastingGameMode* BlastingGameMode = GetWorld()->GetAuthGameMode<ABlastingGameMode>();
		if (BlastingGameMode)
		{
			BlastingGameMode->SetMatchState(MatchState::Settlement);
		}
	}
	else
	{
		ServerUninstall();
	}
}

void ABlastingBomb::ServerUninstall_Implementation()
{
	Uninstall();
}

void ABlastingBomb::OnUninstalled()
{
	if (GetInteractWidget())
	{
		GetInteractWidget()->SetVisibility(false);
	}
}

void ABlastingBomb::OnRep_bIsInstalled()
{
	if (bIsInstalled)
	{
		OnInstalled();
	}
	else
	{
		OnUninstalled();
	}
}
