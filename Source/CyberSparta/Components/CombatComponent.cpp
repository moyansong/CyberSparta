// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "../CyberSparta.h"
#include "../Characters/MyAnimInstance.h"
#include "../Weapons/Weapon.h"
#include "../Weapons/RangedWeapon.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController//MyPlayerController.h"
#include "../HUD/MyHUD.h"
#include "../HUD/WeaponWidget.h"
#include "../GameMode/MyGameMode.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->Controller);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (MyCharacter && MyCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		MyHitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
	}
	
	//Test();
}

void UCombatComponent::Test()
{
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Weapons);
	DOREPLIFETIME(UCombatComponent, WeaponClasses);
	DOREPLIFETIME(UCombatComponent, CurrWeaponIndex);
}

void UCombatComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

}

void UCombatComponent::SetCombatState(ECombatState State)
{
	CombatState = State;
	OnStateChanged();
	if (MyCharacter && !MyCharacter->HasAuthority())
	{
		ServerSetCombatState(State);
	}
}

void UCombatComponent::ServerSetCombatState_Implementation(ECombatState State)
{
	SetCombatState(State);
}

void UCombatComponent::OnRep_CombatState()
{
	OnStateChanged();
}

void UCombatComponent::OnStateChanged()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Idle:
		if (MyCharacter && EquippedWeapon)
		{
			MyCharacter->StopAnimMontage(EquippedWeapon->CharacterFireMontage);
		}
		break;
	case ECombatState::ECS_Reloading:
		SimulateReload();
		break;
	case ECombatState::ECS_Firing:
		SetIsAiming(true);
		break;
	}
}

void UCombatComponent::SpawnDefaultWeapons()
{
	// 在Lobby等地方不生成默认武器,只有Server能获得GameMode
	AMyGameMode* MyGameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (World && MyGameMode && GetOwner() && MyCharacter && MyCharacter->IsAlive())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		APawn* InstigatorPawn = Cast<APawn>(GetOwner());
		SpawnParams.Instigator = InstigatorPawn;

		for (auto WeaponClass : WeaponClasses)
		{
			AWeapon* Weapon = World->SpawnActor<AWeapon>(
				WeaponClass,
				FVector(0.f, 0.f, 10000.f),
				FRotator::ZeroRotator,
				SpawnParams
			);
			if (Weapon)
			{
				AddWeapon(Weapon);
			}
		}

		EquipWeapon(0);
	}
}

void UCombatComponent::OnRep_Weapons()
{
	if (WeaponsNum < Weapons.Num() && Weapons.Num() > 0)
	{
		SetIdleWeapon(Weapons[Weapons.Num() - 1]);
	}
	WeaponsNum = Weapons.Num();
}

void UCombatComponent::AddWeapon(AWeapon* Weapon)
{
	if (!MyCharacter || !Weapon || (Weapons.Num() == WeaponClasses.Num() && WeaponClasses.Contains(Weapon->GetClass()))) return;
	if (MyCharacter->HasAuthority())
	{
		Weapons.AddUnique(Weapon);
		WeaponClasses.AddUnique(Weapon->GetClass());
		SetIdleWeapon(Weapon);
		WeaponsNum = Weapons.Num();
	}
	else
	{
		ServerAddWeapon(Weapon);
	}
}

void UCombatComponent::ServerAddWeapon_Implementation(AWeapon* Weapon)
{
	AddWeapon(Weapon);
}

void UCombatComponent::RemoveWeapon(AWeapon* Weapon)
{
	if (Weapon && MyCharacter && MyCharacter->HasAuthority())
	{
		Weapons.Remove(Weapon);
		WeaponClasses.Remove(Weapon->GetClass());
		WeaponsNum = Weapons.Num();
		EquippedWeapon = nullptr;
		EquipWeapon(0);
	}
}

void UCombatComponent::SetIdleWeapon(AWeapon* Weapon)
{
	if (!Weapon) return;

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->GetMesh())
	{
		Weapon->SetOwner(GetOwner());
		Weapon->SetWeaponState(EWeaponState::EWS_Idle);
		const USkeletalMeshSocket* HandSocket = MyCharacter->GetMesh()->GetSocketByName(FName("hand_rSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(Weapon, MyCharacter->GetMesh());
		}
	}
}

void UCombatComponent::EquipWeapon(int32 Value)
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (!MyCharacter || CombatState != ECombatState::ECS_Idle) return;

	if (Weapons.IsEmpty())
	{
		EquippedWeapon = nullptr;
		MyCharacter->SetAnimationClass();
		return;
	}

	int32 NewIndex = CurrWeaponIndex + Value;
	if (NewIndex < 0) NewIndex = Weapons.Num() - 1;
	else if (NewIndex >= Weapons.Num()) NewIndex = 0;
	
	// 为防止延迟先在客户端做一遍
	CurrWeaponIndex = NewIndex;
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Idle);
		LastEquippedWeapon = EquippedWeapon;
	}
	EquippedWeapon = Weapons[CurrWeaponIndex];
	if (EquippedWeapon)
	{
		EquippedWeapon->SetOwner(GetOwner());
		EquippedWeapon->Equip();
	}
	MyCharacter->SetAnimationClass();

	if (MyCharacter->IsLocallyControlled()) LocalEquipWeapon();
	if (!MyCharacter->HasAuthority()) ServerEquipWeapon(Value);
}

void UCombatComponent::LocalEquipWeapon()
{
	LastFireTime = -100.f;
}

void UCombatComponent::ServerEquipWeapon_Implementation(int32 Value)
{
	EquipWeapon(Value);
}

void UCombatComponent::OnRep_EquippedWeapon(AWeapon* LastWeapon)
{
	if (MyCharacter) MyCharacter->SetAnimationClass();
	if (EquippedWeapon) EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	if (LastWeapon) LastEquippedWeapon = LastWeapon; 
}

void UCombatComponent::OnRep_CurrWeaponIndex()
{
}

void UCombatComponent::ThrowWeapon()
{
	if (!EquippedWeapon || !EquippedWeapon->CanDrop() || !MyCharacter || CombatState != ECombatState::ECS_Idle) return;

	if (MyCharacter->HasAuthority())
	{
		FVector ThrowDirection = MyCharacter->GetFollowCamera() ? MyCharacter->GetFollowCamera()->GetForwardVector() : MyCharacter->GetActorForwardVector();
		EquippedWeapon->Throw(ThrowDirection, 2000.f);
		RemoveWeapon(EquippedWeapon); 
	}
	else
	{
		ServerThrowWeapon();
	}
}

void UCombatComponent::ServerThrowWeapon_Implementation()
{
	ThrowWeapon();
}

void UCombatComponent::SetIsAiming(bool bAiming)
{
	bIsAiming = bAiming; 
	if (MyCharacter && bIsAiming)
	{
		MyCharacter->SetMaxWalkSpeed(false);
	}
}

void UCombatComponent::OnRep_bIsAiming()
{
	if (bIsAiming)
	{
		if (MyCharacter && MyCharacter->GetCharacterMovement())
		{
			MyCharacter->GetCharacterMovement()->MaxWalkSpeed = MyCharacter->AimingJogSpeed;
		}
	}
}

bool UCombatComponent::CanFire()
{
	bool bCanFire = EquippedWeapon &&
					EquippedWeapon->CanFire() &&
					CombatState != ECombatState::ECS_Reloading &&
					MyCharacter && !MyCharacter->GetDisableGameplay() &&
					(EquippedWeapon->CanAutomaticFire() || GetWorld()->GetTimeSeconds() - LastFireTime >= EquippedWeapon->GetFireDelay());
	return bCanFire;
}

void UCombatComponent::FireStart()
{
	if (!CanFire()) return;
	
	SetIsAiming(true);
	Fire();// 按键Release过快可能导致定时器还没触发就关闭了，这里保证至少开火一次
	if (EquippedWeapon->CanAutomaticFire())
	{
		MyCharacter->GetWorldTimerManager().SetTimer(
			FireTimer,
			this,
			&UCombatComponent::Fire,
			EquippedWeapon->GetFireDelay(),
			EquippedWeapon->CanAutomaticFire()
		);
	}
	else
	{
		MyCharacter->GetWorldTimerManager().SetTimer(
			FireTimer,
			this,
			&UCombatComponent::FireFinished,
			EquippedWeapon->GetFireDelay(),
			false
		);
	}
}

void UCombatComponent::FireFinished()
{
	if (CombatState == ECombatState::ECS_Firing)
	{
		SetCombatState(ECombatState::ECS_Idle);
	}
}

void UCombatComponent::Fire()
{
	if (!CanFire()) return;

	LocalFire(MyHitTarget);
	ServerFire(MyHitTarget, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::LocalFire(const FVector& HitTarget)
{
	SimulateFire();
	EquippedWeapon->FireStart(HitTarget);
	LastFireTime = GetWorld()->GetTimeSeconds();
	CrosshairsFireFactor = FMath::Clamp(CrosshairsFireFactor + 2.f, 0.f, 5.f);
}

void UCombatComponent::SimulateFire()
{
	if (EquippedWeapon->CharacterFireMontage)
	{
		MyCharacter->PlayAnimMontage(EquippedWeapon->CharacterFireMontage);
	}
	EquippedWeapon->SimulateFire();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize100& HitTarget, float FireDelay)
{
	SetCombatState(ECombatState::ECS_Firing);
	if (MyCharacter->ShouldMulticastEffect())
	{
		MulticastFire(HitTarget); 
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize100& HitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->GetFireDelay(), FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize100& HitTarget)
{
	if (MyCharacter && MyCharacter->IsLocallyControlled()) return;
	LocalFire(HitTarget);
}

void UCombatComponent::FireStop()
{
	LocalFireStop();
	ServerFireStop();
	if (MyCharacter && EquippedWeapon)
	{
		if (EquippedWeapon->CanAutomaticFire())
		{
			MyCharacter->GetWorldTimerManager().ClearTimer(FireTimer);
			MyCharacter->GetWorldTimerManager().SetTimer(
				FireTimer,
				this,
				&UCombatComponent::FireFinished,
				EquippedWeapon->GetFireDelay(),
				false
			);
		}
	}
}

void UCombatComponent::LocalFireStop()
{
	EquippedWeapon->FireStop();
}

void UCombatComponent::ServerFireStop_Implementation()
{
	MulticastFireStop();
}

void UCombatComponent::MulticastFireStop_Implementation()
{
	if (MyCharacter && MyCharacter->IsLocallyControlled()) return;
	LocalFireStop();
}

void UCombatComponent::TargetStart()
{
	if (!MyCharacter || !EquippedWeapon || !bIsAiming) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
	if (RangedWeapon && RangedWeapon->CanTarget())
	{
		RangedWeapon->TargetStart();
	}
}

void UCombatComponent::TargetStop()
{
	if (!MyCharacter || !EquippedWeapon || !bIsAiming) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
	if (RangedWeapon && RangedWeapon->CanTarget())
	{
		RangedWeapon->TargetStop();
	}
}

bool UCombatComponent::CanReload()
{
	return CombatState == ECombatState::ECS_Idle && EquippedWeapon && EquippedWeapon->CanReload();
}

void UCombatComponent::ReloadStart()
{
	if (!CanReload()) return;
	
	ServerReloadStart();
}

void UCombatComponent::ReloadStop()
{

}

void UCombatComponent::ServerReloadStart_Implementation()
{
	SetCombatState(ECombatState::ECS_Reloading);
	if (EquippedWeapon)
	{
		MyCharacter->GetWorldTimerManager().SetTimer(
			ReloadTimer,
			this,
			&UCombatComponent::ReloadFinished,
			EquippedWeapon->GetReloadDuration(),
			false
		);
	}
}

void UCombatComponent::SimulateReload()
{
	if (!MyCharacter || !EquippedWeapon) return;
	
	if (EquippedWeapon->ReloadMontage)
	{
		MyCharacter->PlayAnimMontage(EquippedWeapon->ReloadMontage);
	}
	EquippedWeapon->SimulateReload();
}

void UCombatComponent::ReloadFinished()
{
	if (MyCharacter && MyCharacter->HasAuthority() && EquippedWeapon)
	{
		SetCombatState(ECombatState::ECS_Idle);
		EquippedWeapon->ReloadFinished();
	}
}

bool UCombatComponent::IsHUDVaild()
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
	if (!WeaponWidget)
	{
		WeaponWidget = MyHUD->WeaponWidget;
		if (!WeaponWidget) return false;
	}
	return true;
}

void UCombatComponent::UpdateHUD(AMyHUD* PlayerHUD)
{
	if (PlayerHUD)
	{
		MyHUD = PlayerHUD;
	}
}

void UCombatComponent::SetHUDWeapon()
{
	SetHUDWeaponAmmo(); 
	SetHUDWeaponImage();
}

void UCombatComponent::SetHUDWeaponAmmo()
{
	if (!IsHUDVaild()) return;

	if (WeaponWidget->AmmoText)
	{
		int32 Ammo = EquippedWeapon ? EquippedWeapon->Ammo : 0;
		int32 MaxAmmo = EquippedWeapon ? EquippedWeapon->MaxAmmo : 0;
		FString AmmoString = FString::Printf(TEXT("%d / %d"), Ammo, MaxAmmo);
		WeaponWidget->AmmoText->SetText(FText::FromString(AmmoString));
	}
}

void UCombatComponent::SetHUDWeaponImage()
{
	if (!IsHUDVaild()) return;

	WeaponWidget->SetHUDWeaponImage(LastEquippedWeapon, EquippedWeapon);
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!MyCharacter || !MyController) return;

	MyHUD = MyHUD ? MyHUD : MyController->GetMyHUD();
	if (MyHUD)
	{
		InitializeHUDCrosshairs();

		// 计算 Corsshairs Spread
		float CrosshairSpread = EquippedWeapon ? EquippedWeapon->CorsshiarsSpreadScale : 1.f;
		FVector Velocity = MyCharacter->GetVelocity();
		Velocity.Z = 0.f;
		CrosshairSpread = Velocity.Size() / 400.f;

		if (MyCharacter->GetCharacterMovement()->IsFalling())
		{
			CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 2.25f, DeltaTime, 2.25f);
		}
		else
		{
			CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 0.f, DeltaTime, 20.f);
		}

		CrosshairsFireFactor = FMath::FInterpTo(CrosshairsFireFactor, 0.f, DeltaTime, 25.f);

		HUDPackage.CrosshairSpread = CrosshairSpread + CrosshairsInAirFactor + CrosshairsFireFactor;
		MyHUD->SetHUDPackage(HUDPackage);
	}
}

void UCombatComponent::InitializeHUDCrosshairs()
{
	if (EquippedWeapon)
	{
		HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
	}
	else
	{
		HUDPackage.CrosshairsCenter = nullptr;
		HUDPackage.CrosshairsLeft = nullptr;
		HUDPackage.CrosshairsRight = nullptr;
		HUDPackage.CrosshairsTop = nullptr;
		HUDPackage.CrosshairsBottom = nullptr;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairsLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairsWorldPosition;
	FVector CrosshairsWorldDirection;
	bool bScreedToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairsLocation,
		CrosshairsWorldPosition,
		CrosshairsWorldDirection
	);

	if (MyCharacter && bScreedToWorld)
	{
		FVector Start = CrosshairsWorldPosition;

		// 避免检测到自己
		float DistanceToMyCharacter = (MyCharacter->GetActorLocation() - Start).Size();
		Start += CrosshairsWorldDirection * (DistanceToMyCharacter + 100.f);

		FVector End = Start + CrosshairsWorldDirection * 10000.f;

		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
		else
		{
			/*DrawDebugSphere(
				GetWorld(),
				HitResult.ImpactPoint,
				12.f,
				12,
				FColor::Red
			); */
		}
		MyHitTarget = HitResult.ImpactPoint;

		if (HitResult.GetActor() && Cast<AMyCharacter>(HitResult.GetActor()))
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::KillReward()
{
	for (auto Weapon : Weapons)
	{
		if (Weapon && (Weapon->GetWeaponType() == EWeaponType::EWT_SpecialWeapon || Weapon == EquippedWeapon))
		{
			Weapon->Drop(); 
		}
	}
}

void UCombatComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	
}