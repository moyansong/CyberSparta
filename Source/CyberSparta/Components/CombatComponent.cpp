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

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
		case ECombatState::ECS_Reloading:
			SimulateReload();
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
	}
}

void UCombatComponent::SetIdleWeapon(AWeapon* Weapon)
{
	if (!Weapon) return;

	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	Weapon->SetOwner(MyCharacter);
	if (MyCharacter && MyCharacter->GetMesh())
	{
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
	if (!MyCharacter || Weapons.IsEmpty())
	{
		EquippedWeapon = nullptr;
		return;
	}

	int32 NewIndex = CurrWeaponIndex + Value;
	if (NewIndex < 0) NewIndex = Weapons.Num() - 1;
	else if (NewIndex >= Weapons.Num()) NewIndex = 0;
	
	// 为防止延迟先在客户端做一遍
	CurrWeaponIndex = NewIndex;
	if (EquippedWeapon) EquippedWeapon->SetWeaponState(EWeaponState::EWS_Idle);
	EquippedWeapon = Weapons[CurrWeaponIndex];
	if (EquippedWeapon)
	{
		EquippedWeapon->SetOwner(MyCharacter);
		EquippedWeapon->Equip();
	}

	if (!MyCharacter->HasAuthority())
	{
		ServerEquipWeapon(Value);
	}
}

void UCombatComponent::ServerEquipWeapon_Implementation(int32 Value)
{
	EquipWeapon(Value);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (!EquippedWeapon) return;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
}

void UCombatComponent::OnRep_CurrWeaponIndex()
{
}

void UCombatComponent::ThrowWeapon()
{
	if (!EquippedWeapon || !MyCharacter) return;
	if (MyCharacter->HasAuthority())
	{
		EquippedWeapon->Drop();
		if (EquippedWeapon->GetMesh() && MyCharacter->GetFollowCamera())
		{
			EquippedWeapon->GetMesh()->AddImpulseAtLocation(
				MyCharacter->GetFollowCamera()->GetForwardVector() * 2000.f, 
				EquippedWeapon->GetActorLocation()
			);
		}
		RemoveWeapon(EquippedWeapon);
		EquippedWeapon = nullptr;
		EquipWeapon(0);
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
}

bool UCombatComponent::CanFire()
{
	return bIsAiming && EquippedWeapon && EquippedWeapon->CanFire() && CombatState != ECombatState::ECS_Reloading && MyCharacter && !MyCharacter->GetDisableGameplay();
}

void UCombatComponent::FireStart()
{
	if (!MyCharacter || !CanFire()) return;

	Fire();// 按键Release过快可能导致定时器还没触发就关闭了，这里保证至少开火一次
	MyCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::Fire,
		EquippedWeapon->GetFireDelay(),
		EquippedWeapon->CanAutomaticFire()
	);
}

void UCombatComponent::FireStop()
{
	if (MyCharacter)
	{
		MyCharacter->GetWorldTimerManager().ClearTimer(FireTimer);
	}
}

void UCombatComponent::Fire()
{
	if (!MyCharacter || !CanFire()) return;

	CrosshairsFireFactor += 1.f;
	CrosshairsFireFactor = FMath::Min(CrosshairsFireFactor, 5.f);
	
	if (EquippedWeapon) EquippedWeapon->LocalFire();
	
	ServerFireStart(MyHitTarget);

	// 若不需要广播开火，则只在本地客户端模拟开火
	if (!MyCharacter->ShouldMulticastEffect())
	{
		SimulateFire();
	}
}

void UCombatComponent::SimulateFire()
{
	if (!MyCharacter || !EquippedWeapon) return;

	if (EquippedWeapon->FireMontage)
	{
		MyCharacter->PlayAnimMontage(EquippedWeapon->FireMontage);
	}
	EquippedWeapon->SimulateFire();
}

void UCombatComponent::ServerFireStart_Implementation(const FVector_NetQuantize& HitTarget)
{
	EquippedWeapon->FireStart(HitTarget);
	CombatState = ECombatState::ECS_Firing;
	if (MyCharacter->ShouldMulticastEffect())
	{
		MulticastFireStart(HitTarget);
	}
}

void UCombatComponent::MulticastFireStart_Implementation(const FVector_NetQuantize& HitTarget)
{
	SimulateFire();
}

void UCombatComponent::TargetStart()
{
	if (!MyCharacter || !EquippedWeapon || !bIsAiming) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
	if (RangedWeapon)
	{
		RangedWeapon->TargetStart();
	}
}

void UCombatComponent::TargetStop()
{
	if (!MyCharacter || !EquippedWeapon || !bIsAiming) return;

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(EquippedWeapon);
	if (RangedWeapon)
	{
		RangedWeapon->TargetStop();
	}
}

void UCombatComponent::ReloadStart()
{
	if (CombatState != ECombatState::ECS_Reloading && EquippedWeapon && EquippedWeapon->CanReload())
	{
		ServerReloadStart();
	}
}

void UCombatComponent::ReloadStop()
{

}

void UCombatComponent::ServerReloadStart_Implementation()
{
	CombatState = ECombatState::ECS_Reloading;
	SimulateReload();

	// 后续用动画通知来代替
	if (MyCharacter)
	{
		MyCharacter->GetWorldTimerManager().SetTimer(
			ReloadTimer,
			this,
			&UCombatComponent::ReloadFinished,
			2.f,
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
		CombatState = ECombatState::ECS_Idle;
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

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!MyCharacter || !MyController) return;

	MyHUD = MyHUD ? MyHUD : MyController->GetMyHUD();
	if (MyHUD)
	{
		float CrosshairSpread;
		if (EquippedWeapon)
		{
			CrosshairSpread = EquippedWeapon->CorsshiarsSpreadScale;
			HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
			HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
		}
		else
		{
			CrosshairSpread = 1.f;
			HUDPackage.CrosshairsCenter = nullptr;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;
		}

		// 计算 Corsshairs Spread
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

