// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "../CyberSparta.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAnimInstance.h"
#include "../HUD/OverHeadWidget.h"
#include "../Weapons/Weapon.h"
#include "../Components/CombatComponent.h"
#include "../Components/BuffComponent.h"
#include "../Components/AttributeComponent.h"
#include "../Components/LagCompensationComponent.h"
#include "../PlayerController/MyPlayerController.h"
#include "../GameMode/MyGameMode.h"
#include "../PlayerStates/MyPlayerState.h"


AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	TurnRateGamepad = 50.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;	// 人物绕Z轴的旋转随着Controller绕Z转
	bUseControllerRotationRoll = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = AimingJogSpeed;
	GetCharacterMovement()->MaxFlySpeed = 2000.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchWalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;// 离玩家的距离
	CameraBoom->SocketOffset = FVector(0.0f, 80.0f, 65.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = true;

	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidgetComponent->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);	//组件不需要向变量那样复制，只需这样设置就行

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	AttributeComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	// 经典数值，为什么这样设置？
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	CurrMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);
}

void AMyCharacter::Initialized()
{
	// 让其只发生一次
	if (!MyController)
	{
		MyController = Cast<AMyPlayerController>(Controller);
		if (AttributeComponent)
		{
			AttributeComponent->SetHUDHealth();
			AttributeComponent->SetHUDShield();
		}
		if (CombatComponent)
		{
			CombatComponent->SetHUDWeaponAmmo();
		}
	}
	if (OverHeadWidgetComponent)
	{
		UOverHeadWidget* OverHeadWidget = Cast<UOverHeadWidget>(OverHeadWidgetComponent->GetUserWidgetObject());
		if (OverHeadWidget)
		{
			OverHeadWidget->ShowPlayerNetRole(this, "Local");
		}
	}
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMyCharacter::ReceiveDamage);
		if (CombatComponent)
		{
			CombatComponent->SpawnDefaultWeapons();
		}
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HideCharacterIfCameraClose();
	Initialized();
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyCharacter::StopJumping);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::InteractStart);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMyCharacter::InteractStop);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyCharacter::CrouchStart);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMyCharacter::CrouchStop);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyCharacter::FireStart);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMyCharacter::FireStop);
	PlayerInputComponent->BindAction("Target", IE_Pressed, this, &AMyCharacter::TargetStart);
	PlayerInputComponent->BindAction("Target", IE_Released, this, &AMyCharacter::TargetStop);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMyCharacter::ReloadStart);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AMyCharacter::ReloadStop);
	PlayerInputComponent->BindAction("ThrowWeapon", IE_Pressed, this, &AMyCharacter::ThrowWeaponStart);
	PlayerInputComponent->BindAction("ThrowWeapon", IE_Released, this, &AMyCharacter::ThrowWeaponStop);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMyCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("EquipWeapon", this, &AMyCharacter::EquipWeapon);

	PlayerInputComponent->BindAction("Test", IE_Pressed, this, &AMyCharacter::Test);
}

void AMyCharacter::Test()
{
	//LagCompensationComponent->Test();
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMyCharacter, OverlappingActor, COND_OwnerOnly);
	DOREPLIFETIME(AMyCharacter, bDisableGameplay);
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->MyCharacter = this;
	}
	if (AttributeComponent)
	{
		AttributeComponent->MyCharacter = this;
	}
	if (BuffComponent)
	{
		BuffComponent->MyCharacter = this;
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->MyCharacter = this;
	}
}

void AMyCharacter::Destroyed()
{
	Super::Destroyed();

}

void AMyCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetMesh())
		{
			CombatComponent->EquippedWeapon->GetMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetMesh())
		{
			CombatComponent->EquippedWeapon->GetMesh()->bOwnerNoSee = false;
		}
	}
}

void AMyCharacter::SetDisableGameplay(bool DisableGameplay)
{
	bDisableGameplay = DisableGameplay;
}

void AMyCharacter::SetShouldMulticastEffect(bool ShouldMulticastEffect)
{
	bShouldMulticastEffect = ShouldMulticastEffect;
}

void AMyCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::JumpStart()
{
	if (bDisableGameplay) return;
	Jump();
}

void AMyCharacter::JumpStop()
{
	if (bDisableGameplay) return;
	StopJumping();
}

void AMyCharacter::CrouchStart()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
		CurrMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}
	else
	{
		Crouch();
		CurrMaxSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
	}
}

void AMyCharacter::CrouchStop()
{
	if (bDisableGameplay) return;
	//UnCrouch();
}

ECombatState AMyCharacter::GetCombatState() const
{
	return CombatComponent ? CombatComponent->CombatState : ECombatState::ECS_MAX;
}

void AMyCharacter::SetIsAiming(bool bAiming)
{
	if (IsAiming() == bAiming) return;

	// 瞄准时不能跑
	// 放在前面，如果按下按键的是客户端，会先在本地修改这个变量，在调用ServerRPC修改，进行复制
	// 这样的好处是客户端按下按键后能在本客户端立刻响应(其他客户端还需要等待复制)，避免网络延迟导致变量复制很慢
	if (bAiming)
	{
		//GetCharacterMovement()->MaxWalkSpeed = AimingJogSpeed;
		GetCharacterMovement()->MaxWalkSpeed = 50.f;
	}
	else
	{

	}

	if (HasAuthority())
	{
		CombatComponent->SetIsAiming(bAiming);
	}
	else
	{
		ServerSetIsAiming(bAiming);
	}

	// 放在后面保证其只在server执行，通过MovementComponent复制到客户端
	// 如果网络有延迟会导致客户端按下对应按键后很久才生效
}

bool AMyCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bIsAiming;
}

FVector AMyCharacter::GetHitTarget() const
{
	return CombatComponent ? CombatComponent->MyHitTarget : FVector();
}

void AMyCharacter::FireStart()
{
	if (!CombatComponent) return;

	SetIsAiming(true);
	CombatComponent->FireStart();
}

void AMyCharacter::FireStop()
{
	if (!CombatComponent) return;
	
	CombatComponent->FireStop();
}

void AMyCharacter::TargetStart()
{
	if (bDisableGameplay) return;
	/*if (IsLocallyControlled() && CombatComponent)
	{
		SetIsAiming(true);
		CombatComponent->TargetStart();
	}*/
}

void AMyCharacter::TargetStop()
{
	if (bDisableGameplay) return;
	/*if (IsLocallyControlled() && CombatComponent)
	{
		CombatComponent->TargetStop();
	}*/
}

void AMyCharacter::SetOverlappingActor(AActor* Actor)
{
	if (OverlappingActor)
	{
		if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
		{
			InteraceActor->SetInteractEffectVisibility(false);
		}
	}
	OverlappingActor = Actor;// 该变量改变后会在客户端执行OnRep_OverlappingActor
	// OnRep_OverlappingActor仅会在OwnerClient进行，下面是为了让Server控制的角色有同样的逻辑
	if (OverlappingActor && IsLocallyControlled())
	{
		if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
		{
			InteraceActor->SetInteractEffectVisibility(true);
		}
	}
}

void AMyCharacter::OnRep_OverlappingActor(AActor* LastActor)
{
	if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
	{
		InteraceActor->SetInteractEffectVisibility(true);
	}
	if (IInteractInterface* LastInteraceActor = Cast<IInteractInterface>(LastActor))
	{
		LastInteraceActor->SetInteractEffectVisibility(false);
	}
}

void AMyCharacter::InteractStart()
{
	if (bDisableGameplay) return;
	IInteractInterface* InteraceActor = OverlappingActor ? Cast<IInteractInterface>(OverlappingActor) : nullptr;
	if (InteraceActor)
	{
		InteraceActor->InteractStart(this);
	}
}

void AMyCharacter::InteractStop()
{
	if (bDisableGameplay) return;
	IInteractInterface* InteraceActor = OverlappingActor ? Cast<IInteractInterface>(OverlappingActor) : nullptr;
	if (InteraceActor)
	{
		InteraceActor->InteractStop(this);
	}
}

void AMyCharacter::ReloadStart()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->ReloadStart();
	}
}

void AMyCharacter::ReloadStop()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->ReloadStop();
	}
}

void AMyCharacter::ThrowWeaponStart()
{
}

void AMyCharacter::ThrowWeaponStop()
{
	if (IsWeaponEquipped())
	{
		CombatComponent->ThrowWeapon();
	}
}

void AMyCharacter::EquipWeapon(float Value)
{
	if (bDisableGameplay || FMath::IsWithin(Value, -0.6f, 0.6f)) return;
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(Value > 0.f ? 1 : (Value < 0.f ? -1 : 0)); 
	}
}

void AMyCharacter::ServerSetIsAiming_Implementation(bool bAiming)
{
	SetIsAiming(bAiming);
}

AWeapon* AMyCharacter::GetEquippedWeapon()
{
	return CombatComponent ? CombatComponent->EquippedWeapon : nullptr;
}

bool AMyCharacter::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

void AMyCharacter::MulticastHit_Implementation()
{
	if (ShouldMulticastEffect())
	{
		PlayHitReactMontage();
	}
}

void AMyCharacter::PlayHitReactMontage()
{
	if (!IsWeaponEquipped()) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* HitReactMontage = GetEquippedWeapon()->HitReactMontage;
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("HitByFwd");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMyCharacter::UpdateHUD(AMyHUD* PlayerHUD)
{
	if (AttributeComponent)
	{
		AttributeComponent->UpdateHUD(PlayerHUD);
	}
}

void AMyCharacter::SimProxiesTurn()
{
	if (!IsWeaponEquipped()) return;

	if (GetLocalRole() <= ENetRole::ROLE_SimulatedProxy)
	{
		// 去看视频里有关Turn的
	}
}

bool AMyCharacter::IsAlive()
{
	return AttributeComponent && AttributeComponent->IsAlive();
}

void AMyCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (AttributeComponent)
	{
		AttributeComponent->ReceiveDamage(DamageActor, Damage, DamageType, InstigatorController, DamageCauser);
	}
}

void AMyCharacter::Elim()
{

}

void AMyCharacter::SimulateElim()
{
	if (AttributeComponent && AttributeComponent->ElimMontage)
	{
		PlayAnimMontage(AttributeComponent->ElimMontage);
	}
}

void AMyCharacter::KillReward()
{
	if (IsWeaponEquipped())
	{
		CombatComponent->EquippedWeapon->Drop();
	}
}