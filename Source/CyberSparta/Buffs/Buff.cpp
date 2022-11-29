// Fill out your copyright notice in the Description page of Project Settings.


#include "Buff.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "../CyberSparta.h"
#include "../Characters/MyCharacter.h"
#include "../Components/BuffComponent.h"

ABuff::ABuff()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(40.f);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	BuffEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	BuffEffectComponent->SetupAttachment(RootComponent);

	PickupWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidgetComponent->SetupAttachment(RootComponent);
}

void ABuff::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuff, BuffState);
}

void ABuff::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SetSphereCollision(true);
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABuff::OnSphereBeginOverlap);
		SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ABuff::OnSphereEndOverlap);
	}

	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(false);
	}

	if (BuffEffectComponent && WorldEffect)
	{
		BuffEffectComponent->SetAsset(WorldEffect);
	}
}

void ABuff::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && BuffState == EBuffState::EBS_Effective)
	{
		TickEffective(DeltaTime);
		CurrBuffDuration += DeltaTime;
		if (CurrBuffDuration >= BuffDuration)
		{
			SetBuffState(EBuffState::EBS_Destroyed);
		}
	}
}

void ABuff::SetSphereCollision(bool bCanOverlapWithPawn)
{
	if (!SphereComponent) return;

	if (bCanOverlapWithPawn)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void ABuff::SetInteractEffectVisibility(bool bVisibility)
{
	if (PickupWidgetComponent)
	{
		PickupWidgetComponent->SetVisibility(bVisibility);
	}
}

void ABuff::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->SetOverlappingActor(this);
	}
}

void ABuff::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMyCharacter* OtherCharacter = Cast<AMyCharacter>(OtherActor);
	if (OtherCharacter)
	{
		OtherCharacter->SetOverlappingActor(nullptr);
	}
}

void ABuff::InteractStart(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter)
	{
		
	}
}

void ABuff::InteractStop(AActor* InteractActor)
{
	if (!InteractActor) return;

	AMyCharacter* InteractCharacter = Cast<AMyCharacter>(InteractActor);
	if (InteractCharacter && InteractCharacter->GetBuffComponent())
	{
		InteractCharacter->GetBuffComponent()->AddBuff(this);
	}
}

void ABuff::SetBuffState(EBuffState State)
{
	BuffState = State;
	OnStateChanged();
}

void ABuff::OnRep_BuffState()
{
	OnStateChanged();
}


void ABuff::OnStateChanged()
{
	switch (BuffState)
	{
	case EBuffState::EBS_Initial:
		break;
	case EBuffState::EBS_Effective:
		OnEffective();
		break;
	case EBuffState::EBS_Paused:
		break;
	case EBuffState::EBS_Destroyed:
		Destroy();
		break;
	}
}

void ABuff::OnEffective()
{
	InitialLifeSpan = 0.f;
	SetInteractEffectVisibility(false);
	SetSphereCollision(false);
	SpawnEmitter(DestroyEffect);
	BuffEffectComponent->SetAsset(AttachEffect);
}

void ABuff::Destroyed()
{
	SpawnEmitter(DestroyEffect);
	MyCharacter = MyCharacter ? MyCharacter : Cast<AMyCharacter>(GetOwner());
	if (MyCharacter && MyCharacter->IsAlive())
	{
		SpawnEmitter(DestroyEffect);
		if (MyCharacter->GetBuffComponent() && HasAuthority())
		{
			MyCharacter->GetBuffComponent()->RemoveBuff(this);
		}
	}
	Super::Destroyed();
}

void ABuff::TickEffective(float DeltaTime)
{
	
}

void ABuff::SetCurrBuffDuration(float Duration)
{
	BuffDuration = Duration;
}

void ABuff::SpawnEmitter(UNiagaraSystem* Effect)
{
	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			Effect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}


