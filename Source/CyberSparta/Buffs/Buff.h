// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/InteractInterface.h"
#include "Buff.generated.h"

class USphereComponent;
class UWidgetComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class AMyCharacter;

UENUM(BlueprintType)
enum class EBuffState : uint8
{
	EBS_Initial UMETA(DisplayName = "Initial"),// 刚初始化在世界里
	EBS_Effective UMETA(DisplayName = "Effective"),// 正在生效的Buff
	EBS_Paused UMETA(DisplayName = "Paused"),// 暂停生效的Buff
	EBS_Destroyed UMETA(DisplayName = "Destroyed"),// 停止生效的Buff
	EBS_MAX UMETA(DisplayName = "DefaultMax") // 这个是为了告诉别人这个枚举有几个(枚举是整数)
};

UCLASS()
class CYBERSPARTA_API ABuff : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	ABuff();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//--------------------------------------------Functions-------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	void SetSphereCollision(bool bCanOverlapWithPawn);

	virtual void SetInteractEffectVisibility(bool bVisibility) override;

	virtual void InteractStart(AActor* InteractActor) override;
	virtual void InteractStop(AActor* InteractActor) override;

	void SetBuffState(EBuffState State);
	
	virtual void OnStateChanged();

	virtual void OnEffective();// 状态变为OnEffective时的逻辑，只执行一次的Buff功能写在这，Server和Client都会调用

	UFUNCTION()
	virtual void TickEffective(float DeltaTime); // 每Tick都要执行Buff的功能写在这里,会在Server的Tick里调用

	void SetCurrBuffDuration(float Duration);

	void SpawnEmitter(UNiagaraSystem* Effect);

protected:
	virtual void BeginPlay() override;

//--------------------------------------------Callbacks-------------------------------------------------------------
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

//--------------------------------------------Parameters-------------------------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_BuffState, VisibleAnywhere, Category = Parameter)
	EBuffState BuffState = EBuffState::EBS_Initial;
	UFUNCTION()
	void OnRep_BuffState();

	UPROPERTY()
	AMyCharacter* MyCharacter;
	
	float BaseTurnRate = 45.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float BuffDuration = 10.f;	// Buff持续时间

	float CurrBuffDuration = 0.f;	// Buff已经生效的时间

//--------------------------------------------Emitters-------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = Effect)
	UNiagaraSystem* WorldEffect;	// 在世界中的特效
	
	UPROPERTY(EditAnywhere, Category = Effect)
	UNiagaraSystem* AttachEffect;	// 在人身上的特效

	UPROPERTY(EditAnywhere, Category = Effect)
	UNiagaraSystem* DestroyEffect;	// Buff失效时的特效
private:
//--------------------------------------------Components-------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UWidgetComponent* InteractWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UNiagaraComponent* BuffEffectComponent;
};
