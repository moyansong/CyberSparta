// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "../CyberSparta.h"
#include "../Characters/MyAnimInstance.h"
#include "../Weapons/Weapon.h"
#include "../Weapons/RangedWeapon.h"
#include "../Characters/MyCharacter.h"
#include "../PlayerController//MyPlayerController.h"
#include "../HUD/MyHUD.h"

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
}

void UCombatComponent::EquipWeaponStart(AWeapon* Weapon)
{
	if (!MyCharacter || !Weapon)
	{
		return;
	}

	// Server会负责所有人的Equip，EquippedWeapon会复制到客户端
	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(MyCharacter);
	EquippedWeapon->EquipWeaponStart();
	const USkeletalMeshSocket* HandSocket = MyCharacter->GetMesh()->GetSocketByName(FName("hand_rSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, MyCharacter->GetMesh());
	}
}

void UCombatComponent::EquipWeaponStop(AWeapon* Weapon)
{
}

void UCombatComponent::FireStart()
{
	if (!MyCharacter || !EquippedWeapon || !bIsAiming) return;

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
	if (!MyCharacter || !EquippedWeapon || !bIsAiming || !CanFire()) return;

	CrosshairsFireFactor += 1.f;
	CrosshairsFireFactor = FMath::Min(CrosshairsFireFactor, 5.f);

	ServerFireStart(MyHitTarget);

	// 若不需要广播开火，则只在本地客户端模拟开火
	if (!MyCharacter->ShouldMulticastFire())
	{
		SimulateFire();
	}
}

void UCombatComponent::SimulateFire()
{
	if (!MyCharacter || !EquippedWeapon) return;

	MyCharacter->PlayAnimMontage(EquippedWeapon->FireMontage);
	EquippedWeapon->SimulateFire();
}

void UCombatComponent::ServerFireStart_Implementation(const FVector_NetQuantize& HitTarget)
{
	MulticastFireStart(HitTarget);
}

void UCombatComponent::MulticastFireStart_Implementation(const FVector_NetQuantize& HitTarget)
{
	if (!MyCharacter || !EquippedWeapon) return;

	EquippedWeapon->FireStart(HitTarget);
	if (MyCharacter->ShouldMulticastFire())
	{
		SimulateFire();
	}
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

void UCombatComponent::SetIsAiming(bool bAiming)
{
	bIsAiming = bAiming;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	// 变量应该尽量少的复制，在已经复制的变量调用回调时可以在这里设置一些客户端的其他变量，避免复制变量
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!MyCharacter || !MyCharacter->Controller) return;

	MyController = MyController == nullptr ? Cast<AMyPlayerController>(MyCharacter->Controller) : MyController;
	if (MyController)
	{
		MyHUD = MyHUD == nullptr ? Cast<AMyHUD>(MyController->GetHUD()) : MyHUD;
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

