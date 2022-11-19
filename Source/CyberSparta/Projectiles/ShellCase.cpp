// Fill out your copyright notice in the Description page of Project Settings.


#include "ShellCase.h"

AShellCase::AShellCase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);// 开了物理模拟要想触发Hit则要设置true，蓝图中是Simulation Grenates Hit Events
	MeshComponent->SetEnableGravity(true);
}

void AShellCase::BeginPlay()
{
	Super::BeginPlay();
	
	MeshComponent->AddImpulse(GetActorForwardVector() * ShellEjectionImpluse);
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

