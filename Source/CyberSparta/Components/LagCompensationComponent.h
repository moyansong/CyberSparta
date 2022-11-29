// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AMyCharacter;
class AMyPlayerController;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

// 记录一帧里人物所有HitBox的信息
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSPARTA_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class AMyCharacter;
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//---------------------------------------------Functions-------------------------------------------------------------
	void Test();

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color = FColor::Red);

	// HitCharacter是被打的Character
	void ServerSideRewind(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; // 保存多长时间的FramePackage

	TDoubleLinkedList<FFramePackage> FrameHistory;// 保存一段时间内的所有HitBox的位置,越靠头时间越近(大)
};
