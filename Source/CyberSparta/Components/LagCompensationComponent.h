// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AMyCharacter;
class AMyPlayerController;
class AWeapon;

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

	UPROPERTY()
	AMyCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed; // 是否打中

	UPROPERTY()
	bool bHeadShot;	// 是否打中头
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

	FFramePackage GetFrameToCheck(AMyCharacter* HitCharacter, float HitTime);
	// HitCharacter是被打的Character，这几个参数都是客户端传上来的,找到对应FFramePackage在调用Confirm
	// 仅供Hitscan武器调用
	FServerSideRewindResult ServerSideRewind(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	// Projectile用的
	FServerSideRewindResult ProjectileServerSideRewind(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	// 外部调用，检查是否打中人，打中则直接施加伤害
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(); // 保存一段时间的FramePackage
	void SaveFramePackage(FFramePackage& Package);// 保存一帧的Package，通过Package传出

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	// 根据Package信息确定HitCharacter是否被打中（LineTrace）,HitscanWeapon用
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	// 根据Package信息确定HitCharacter是否被打中（子弹轨迹预测PredictProjectilePath）,子弹用
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AMyCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	void SetCharacterMeshCollision(AMyCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	// 把当前人物box的位置移到Package里Box的位置
	void MoveBoxs(AMyCharacter* HitCharacter, const FFramePackage& Package);

	void ResetHitBoxs(AMyCharacter* HitCharacter, const FFramePackage& Package);

	// 获取当前帧的HitCharacter的Box信息，通过OutFramePackage传出
	void CacheBoxPositions(AMyCharacter* HitCharacter, FFramePackage& OutFramePackage);
//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY()
	AMyCharacter* MyCharacter;
	UPROPERTY()
	AMyPlayerController* MyController;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; // 保存多长时间的FramePackage

	TDoubleLinkedList<FFramePackage> FrameHistory;// 保存一段时间内的所有HitBox的位置,越靠头时间越近(大)
};
