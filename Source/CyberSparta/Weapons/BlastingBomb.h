// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpecialWeapon.h"
#include "BlastingBomb.generated.h"

class UUserWidget;

UCLASS()
class CYBERSPARTA_API ABlastingBomb : public ASpecialWeapon
{
	GENERATED_BODY()

//---------------------------------------------Functions-------------------------------------------------------------
public:
	ABlastingBomb();
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	virtual void OnIdled() override;

	virtual void OnRep_Owner() override;

	void Initialize(FVector CenterOfBlastingZone, float RadiusOfBlastingZone);

	// 红队用互动键捡起，开火键安炸弹，蓝队用互动拆炸弹
	virtual void InteractStart(AActor* InteractActor) override;
	virtual void InteractStop(AActor* InteractActor) override;

	virtual void FireStart(const FVector& HitTarget) override;
	virtual void FireStop() override;

	virtual bool CanFire() override;

	void Install();

	void OnInstalled();

	void Uninstall();

	void OnUninstalled();

	FORCEINLINE float GetBlastingDelay() const { return BlastingDelay; }

protected:
	virtual void BeginPlay() override;

	// 拆弹或安弹的人不应该移动/开火，在Tick里检查
	void CheckOwnerState();

	UFUNCTION(Server, Reliable)
	void ServerInstall();

	UFUNCTION(Server, Reliable)
	void ServerUninstall();

	void SetInstallLocation();

	void BindCallbackOfHealthChanged(bool bShouldBind);

	// 人受到伤害时不能拆/装炸弹
	UFUNCTION()
	void OnHealthChanged(AActor* DamageActor, float OldHealth, float NewHealth);
//---------------------------------------------Parameters-------------------------------------------------------------
private:
	FTimerHandle InstallTimer;
	FTimerHandle UninstallTimer;
	
	// 安装这个炸弹需要按住开火键的秒数
	UPROPERTY(EditAnywhere, Category = Parameter)
	float InstallDuration = 5.f;

	// 拆除这个炸弹需要按住互动键的秒数
	UPROPERTY(EditAnywhere, Category = Parameter)
	float UninstallDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	float BlastingDelay = 60.f;

	UPROPERTY(Replicated)
	FVector BlastingZoneCenter;

	UPROPERTY(Replicated)
	float BlastingZoneRadius = 10000.f;

	UPROPERTY(ReplicatedUsing = OnRep_bIsInstalled)
	bool bIsInstalled = false;
	UFUNCTION()
	void OnRep_bIsInstalled();
};
