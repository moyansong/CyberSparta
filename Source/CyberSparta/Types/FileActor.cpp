// Fill out your copyright notice in the Description page of Project Settings.


#include "FileActor.h"

// Sets default values
AFileActor::AFileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFileActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

