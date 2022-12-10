// Fill out your copyright notice in the Description page of Project Settings.


#include "ShellCase.h"
#include "../CyberSparta.h"

AShellCase::AShellCase()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 3.f;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);// 开了物理模拟要想触发Hit则要设置true，蓝图中是Simulation Grenates Hit Events
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

void AShellCase::BeginPlay()
{
	Super::BeginPlay();
	
	FRotator Rotation = GetOwner() ? GetOwner()->GetActorRotation() : FRotator::ZeroRotator;
	Rotation.Pitch += 45.f;
	Rotation.Yaw += 90.f;
	MeshComponent->AddImpulse(Rotation.Vector() * 10.f);
	MeshComponent->OnComponentHit.AddDynamic(this, &AShellCase::OnHit);
}

void AShellCase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AShellCase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

