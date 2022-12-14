// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "../CyberSparta.h"
#include "../Weapons/HitScanWeapon.h"
#include "../Weapons/RangedWeapon.h"
#include "../Weapons/Shotgun.h"
#include "../Projectiles/Projectile.h"
#include "../Characters/MyCharacter.h"
#include "../Components/AttributeComponent.h"
#include "../GameFramework/HeadShotDamageType.h"
#include "../GameFramework/TrunkShotDamageType.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

void ULagCompensationComponent::Test()
{
	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package);
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (!MyCharacter || !MyCharacter->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter)
	{
		Package.Character = MyCharacter;
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : MyCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::CacheBoxPositions(AMyCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (!HitCharacter) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::MoveBoxs(AMyCharacter* HitCharacter, const FFramePackage& Package)
{
	if (!HitCharacter) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxs(AMyCharacter* HitCharacter, const FFramePackage& Package)
{
	if (!HitCharacter) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AMyCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		!HitCharacter || !HitCharacter->GetLagCompensationComponent() ||
		!HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() ||
		!HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail();
	if (bReturn) return FFramePackage();

	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	// ???????????????HitTime????????????Older???????????????????????????Younger???????????????????????????Package???????????????
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime && Older->GetNextNode())
	{
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}


FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<AMyCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (AMyCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	AHitScanWeapon* HitScanWeapon = Cast<AHitScanWeapon>(DamageCauser);// ??????????????????EquippedWeapon
	if (MyCharacter && HitCharacter && Confirm.bHitConfirmed && HitScanWeapon)
	{
		TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
		if (Confirm.bHeadShot)
		{
			DamageTypeClass = UHeadShotDamageType::StaticClass();
		}
		else if (Confirm.bTrunkShot)
		{
			DamageTypeClass = UTrunkShotDamageType::StaticClass();
		}

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Confirm.bHeadShot ? HitScanWeapon->GetHeadShotDamage() : HitScanWeapon->GetDamage(),
			MyCharacter->Controller,
			DamageCauser,
			DamageTypeClass
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<AMyCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	AShotgun* Shotgun = Cast<AShotgun>(DamageCauser);
	if (!Shotgun || !MyCharacter) return;

	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	for (auto& HitCharacter : HitCharacters)
	{
		if (!HitCharacter) continue;
		float HeadShotDamage = 0.f, BodyDamage = 0.f, TrunkDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			HeadShotDamage = Confirm.HeadShots[HitCharacter] * Shotgun->GetHeadShotDamage();
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			BodyDamage = Confirm.BodyShots[HitCharacter] * Shotgun->GetDamage();
		}
		if (Confirm.TrunkShots.Contains(HitCharacter))
		{
			TrunkDamage = Confirm.TrunkShots[HitCharacter] * Shotgun->GetDamage();
			if (HitCharacter->GetAttributeComponent())
			{
				float DamageToShield = HitCharacter->GetAttributeComponent()->CalculateShiledReceivedDamage(TrunkDamage);
				HitCharacter->GetAttributeComponent()->DecreaseShield(DamageToShield);
				TrunkDamage -= DamageToShield;
			}
		}

		TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
		if (HeadShotDamage >= BodyDamage + TrunkDamage && HeadShotDamage >= HitCharacter->GetHealth())
		{
			DamageTypeClass = UHeadShotDamageType::StaticClass();
		}

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			HeadShotDamage + BodyDamage + TrunkDamage,
			MyCharacter->Controller,
			DamageCauser,
			DamageTypeClass
		);
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	if (MyCharacter && MyCharacter->GetEquippedWeapon() && HitCharacter && Confirm.bHitConfirmed)
	{
		ARangedWeapon* MyWeapon = Cast<ARangedWeapon>(MyCharacter->GetEquippedWeapon());
		if (MyWeapon)
		{
			TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
			if (Confirm.bHeadShot)
			{
				DamageTypeClass = UHeadShotDamageType::StaticClass();
			}
			else if (Confirm.bTrunkShot)
			{
				DamageTypeClass = UTrunkShotDamageType::StaticClass();
			}

			AProjectile* DefaultProjectile = MyWeapon->GetDefaultProjectile();
			if (DefaultProjectile)
			{
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					Confirm.bHeadShot ? DefaultProjectile->GetHeadShotDamage() : DefaultProjectile->GetDamage(),
					MyCharacter->Controller,
					MyWeapon,
					DamageTypeClass
				);
			}
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterFramePackage;
	InterFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		// ???????????????????????????????????????????????????????????????????????????????????????HitTime????????????DeltaTime??????????????????1.f
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (!HitCharacter) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxs(HitCharacter, Package);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit) // ????????????
		{
			ResetHitBoxs(HitCharacter, CurrentFrame); 
			SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true, false };
		}
		else // ?????????????????????????????????????????????
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxs(HitCharacter, CurrentFrame);
				SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); 
				return FServerSideRewindResult{ true, false, IsTrunkShot(ConfirmHitResult) };
			}
		}
	}
	// ??????????????????
	ResetHitBoxs(HitCharacter, CurrentFrame);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& Packages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Package : Packages)
	{
		if (!Package.Character) return FShotgunServerSideRewindResult();
	}
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Package : Packages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Package.Character;
		CacheBoxPositions(Package.Character, CurrentFrame);
		MoveBoxs(Package.Character, Package);
		SetCharacterMeshCollision(Package.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Package : Packages)
	{
		UBoxComponent* HeadBox = Package.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}
	UWorld* World = GetWorld();
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			AMyCharacter* HitCharacter = Cast<AMyCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (ShotgunResult.HeadShots.Contains(HitCharacter))
				{
					++ShotgunResult.HeadShots[HitCharacter];
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(HitCharacter, 1);
				}
			}
		}
	}

	for (auto& Package : Packages)
	{
		for (auto& HitBoxPair : Package.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Package.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			AMyCharacter* HitCharacter = Cast<AMyCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (IsTrunkShot(ConfirmHitResult))
				{
					if (ShotgunResult.TrunkShots.Contains(HitCharacter))
					{
						++ShotgunResult.TrunkShots[HitCharacter];
					}
					else
					{
						ShotgunResult.TrunkShots.Emplace(HitCharacter, 1);
					}
				}
				else
				{
					if (ShotgunResult.BodyShots.Contains(HitCharacter))
					{
						++ShotgunResult.BodyShots[HitCharacter];
					}
					else
					{
						ShotgunResult.BodyShots.Emplace(HitCharacter, 1);
					}
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxs(Frame.Character, Frame);
		SetCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	return ShotgunResult;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (!HitCharacter) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxs(HitCharacter, Package);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	// ????????????????????????????????????????????????
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PredictParams;
	PredictParams.bTraceWithCollision = true;
	PredictParams.LaunchVelocity = InitialVelocity;
	PredictParams.MaxSimTime = MaxRecordTime;
	PredictParams.ProjectileRadius = 5.f;
	PredictParams.SimFrequency = 15.f;
	PredictParams.StartLocation = TraceStart;
	PredictParams.TraceChannel = ECC_HitBox;
	PredictParams.ActorsToIgnore.Add(GetOwner());
	//PredictParams.DrawDebugTime = 5.f;
	//PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	if (PredictResult.HitResult.bBlockingHit)// ????????????
	{
		ResetHitBoxs(HitCharacter, CurrentFrame);
		SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true, true, false };
	}
	else //?????????????????????????????????????????????
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
		if (PredictResult.HitResult.bBlockingHit) // ?????????????????????
		{
			ResetHitBoxs(HitCharacter, CurrentFrame);
			SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false, IsTrunkShot(PredictResult.HitResult) };
		}
	}
	// ??????????????????
	ResetHitBoxs(HitCharacter, CurrentFrame);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false, false };
}

void ULagCompensationComponent::SetCharacterMeshCollision(AMyCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

bool ULagCompensationComponent::IsTrunkShot(const FHitResult& HitResult)
{
	if (!HitResult.GetComponent()) return false;

	FName HitBone = HitResult.GetComponent()->GetFName();
	return HitBone == FName("spine_01") || HitBone == FName("spine_02") || HitBone == FName("spine_03");
}
