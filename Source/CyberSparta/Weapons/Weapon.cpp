// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimationAsset.h"
#include "Components/TextBlock.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../HUD/InteractWidget.h"
#include "../PlayerController/MyPlayerController.h"
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

	InteractWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidgetComponent->SetupAttachment(RootComponent);

	SetWeaponState(EWeaponState::EWS_Initialized);
	
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

	if (InteractWidgetComponent)
	{
		InteractWidgetComponent->SetVisibility(false);
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
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnRep_Owner()
{
	// 不要在这里做别的事，想写东西去子类里写，因为有些子类的Owner可能并不是持有这个武器的角色

	Super::OnRep_Owner();

	// MyCharacter 和 MyController都是本地变量，以Owner为准
	// Server上的设置会在SetIdle里进行
	MyCharacter = Cast<AMyCharacter>(GetOwner());
	if (MyCharacter) MyController = Cast<AMyPlayerController>(MyCharacter->GetController());
}

void AWeapon::SetSphereCollision(bool bCanOverlapWithPawn)
{
	if (!SphereComponent) return;

	if (bCanOverlapWithPawn)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void AWeapon::SetMeshSimulatePhysics(UPrimitiveComponent* Mesh, bool bSimulatePhysics)
{
	if (!Mesh) return;

	if (bSimulatePhysics)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);	// 如果Mesh穿过障碍物，很有可能是障碍物的问题，可能是太薄，或者是障碍物地形也有可能穿过
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		//Pickup(InteractCharacter);
	}
}

void AWeapon::InteractStop(AActor* InteractActor)
{
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
	}
}

void AWeapon::SetInteractEffectVisibility(bool bVisibility)
{
	if (InteractWidgetComponent)
	{
		InteractWidgetComponent->SetVisibility(bVisibility);
	}
}

void AWeapon::SetInteractText(const FString& InteractString)
{
	if (InteractWidgetComponent)
	{
		UInteractWidget* InteractWidget = Cast<UInteractWidget>(InteractWidgetComponent->GetWidget());
		if (InteractWidget && InteractWidget->InteractText)
		{
			InteractWidget->InteractText->SetText(FText::FromString(InteractString)); 
		}
	}
}

bool AWeapon::CanReload()
{
	return bCanReload;
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

bool AWeapon::UseRightHandRotation()
{
	return bUseRightHandRotation;
}

bool AWeapon::CanFire()
{
	return Ammo >= AmmoCostPerFire;
}

void AWeapon::FireStart(const FVector& HitTarget)
{
	SpendRound();
}

void AWeapon::FireStop()
{
	
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
	if (!CanDrop()) return;

	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	MeshComponent->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	MyCharacter = nullptr;
	MyController = nullptr;
	Ammo = MaxAmmo;
}

void AWeapon::Throw(FVector ThrowDirection, float Force)
{
	if (!CanDrop()) return;

	Drop();
	if (MeshComponent)
	{
		MeshComponent->AddImpulseAtLocation(ThrowDirection * Force, GetActorLocation());
	}
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
	case EWeaponState::EWS_Initialized:
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
	if (!SphereComponent || !MeshComponent) return;

	InitialLifeSpan = 0.f;
	SetHUDWeapon();
	SetSphereCollision(false);
	SetInteractEffectVisibility(false);
	MeshComponent->SetVisibility(true);
	MeshComponent->SetRenderCustomDepth(false);
	SetMeshSimulatePhysics(MeshComponent, false);
}

void AWeapon::OnDropped()
{
	if (!SphereComponent || !MeshComponent) return;

	SetSphereCollision(HasAuthority());
	MeshComponent->SetVisibility(true);
	MeshComponent->SetRenderCustomDepth(true); 
	SetMeshSimulatePhysics(MeshComponent, true);
}

void AWeapon::OnIdled()
{
	if (!SphereComponent || !MeshComponent) return;

	InitialLifeSpan = 0.f;
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter) MyController = MyController ? MyController : Cast<AMyPlayerController>(MyCharacter->GetController());
	SetSphereCollision(false);
	SetInteractEffectVisibility(false);
	MeshComponent->SetVisibility(false);
	MeshComponent->SetRenderCustomDepth(false);
	SetMeshSimulatePhysics(MeshComponent, false);
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
	if (HasAuthority())
	{
		SetAmmo(Ammo - AmmoCostPerFire);
	}
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

void AWeapon::SetHUDWeapon()
{
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->GetCombatComponent())
	{
		MyCharacter->GetCombatComponent()->SetHUDWeapon();
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

