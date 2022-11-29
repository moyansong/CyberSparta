// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimationAsset.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Components/CombatComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	SetRootComponent(MeshComponent);

	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRenderCustomDepth(true);	// 会在Mesh的边缘用颜色描边，需要一些设置去看Blaster149
	MeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere")); // 不能叫SphereCollision?
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(40.f);
	// 在客户端设为无碰撞，在Server端设置为可以碰撞, Server管理所有Weapond的碰撞
	SetSphereCollision(false);

	PickupWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidgetComponent->SetupAttachment(RootComponent);

	SetWeaponState(EWeaponState::EWS_Initial);
	
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 在客户端设为无碰撞，在Server端设置为可以碰撞, 只有Server会发生Overlap
		// 如果在所有客户端都有碰撞，捡枪不会有问题，Widget显示会有问题
		// 两个人同时与枪重叠，一个人离开，结束重叠，两个人都会触发OnSphereEndOverlap事件，同时关闭Widget
		SetSphereCollision(true);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereBeginOverlap);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	MyCharacter = GetOwner() ? Cast<AMyCharacter>(GetOwner()) : nullptr;
}

void AWeapon::SetSphereCollision(bool bCanOverlapWithPawn)
{
	if (!SphereComponent) return;

	if (bCanOverlapWithPawn)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void AWeapon::SetMeshSimulatePhysics(bool bSimulatePhysics)
{
	if (!MeshComponent) return;

	if (bSimulatePhysics)
	{
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->SetEnableGravity(true);	// 如果Mesh穿过障碍物，很有可能是障碍物的问题，可能是太薄，或者是障碍物地形也有可能穿过
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetEnableGravity(false);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->SetOverlappingActor(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->SetOverlappingActor(nullptr);
	}
}

void AWeapon::InteractStart(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		//Pickup(InteractCharacter);
	}
}

void AWeapon::InteractStop(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		Pickup(InteractCharacter);
	}
}

void AWeapon::Pickup(AMyCharacter* PickCharacter)
{
	if (PickCharacter && PickCharacter->GetCombatComponent())
	{
		PickCharacter->GetCombatComponent()->AddWeapon(this);
		MyCharacter = PickCharacter;
	}
}

void AWeapon::SetInteractEffectVisibility(bool bVisibility)
{
	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(bVisibility);
	}
}

void AWeapon::ReloadStart()
{
}

void AWeapon::ReloadStop()
{
}

void AWeapon::SimulateReload()
{
	if (ReloadAnimation && MeshComponent)
	{
		MeshComponent->PlayAnimation(ReloadAnimation, false);
	}
}

void AWeapon::ReloadFinished()
{
	SetAmmo(MaxAmmo);
	//ClientUpdateAmmo(Ammo);
}

bool AWeapon::CanFire()
{
	return Ammo > 0;
}

void AWeapon::FireStart(const FVector& HitTarget)
{
	SpendRound();
}

void AWeapon::FireStop()
{
}

void AWeapon::LocalFire()
{
	//SpendRound();// 如果Ammo是复制的则不需要
}

void AWeapon::SimulateFire()
{
	if (FireAnimation && MeshComponent)
	{
		MeshComponent->PlayAnimation(FireAnimation, false);
	}
}

void AWeapon::Equip()
{
	SetWeaponState(EWeaponState::EWS_Equipped);
}

void AWeapon::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	MeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	MyCharacter = nullptr;
	Ammo = MaxAmmo;
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnStateChanged();
}

void AWeapon::OnRep_WeaponState()
{
	OnStateChanged();
}

void AWeapon::OnStateChanged()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	case EWeaponState::EWS_Idle:
		OnIdled();
		break;
	}
}

void AWeapon::OnEquipped()
{
	if (!MeshComponent || !SphereComponent) return;

	InitialLifeSpan = 0.f;
	SetHUDWeaponAmmo();
	SetInteractEffectVisibility(false);
	SetSphereCollision(false);
	SetMeshSimulatePhysics(false);
	MeshComponent->SetRenderCustomDepth(false);
	MeshComponent->SetVisibility(true);
}

void AWeapon::OnDropped()
{
	if (!MeshComponent || !SphereComponent) return;

	if (HasAuthority())
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	SetMeshSimulatePhysics(true);
	MeshComponent->SetRenderCustomDepth(true);
}

void AWeapon::OnIdled()
{
	if (!MeshComponent || !SphereComponent) return;
	
	InitialLifeSpan = 0.f;
	SetInteractEffectVisibility(false);
	SetSphereCollision(false);
	SetMeshSimulatePhysics(false);
	MeshComponent->SetRenderCustomDepth(false);
	MeshComponent->SetVisibility(false);
}

void AWeapon::SetAmmo(int32 AmmoToSet)
{
	if (Ammo != AmmoToSet)
	{
		Ammo = FMath::Clamp(AmmoToSet, 0, MaxAmmo);
		SetHUDWeaponAmmo();
	}
}

void AWeapon::SpendRound()
{
	SetAmmo(Ammo - AmmoCostPerFire);
	//if (HasAuthority())
	//{
	//	ClientUpdateAmmo(Ammo);// 不复制Ammo，用RPC更新Ammo
	//}
	//else
	//{
	//	++AmmoSequence;
	//}
}

void AWeapon::SetHUDWeaponAmmo()
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		MyCharacter->GetCombatComponent()->SetHUDWeaponAmmo();
	}
}

void AWeapon::OnRep_Ammo()
{
	SetHUDWeaponAmmo();
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	--AmmoSequence;
	SetAmmo(ServerAmmo - AmmoSequence * AmmoCostPerFire);
}

