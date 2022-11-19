// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "../CyberSparta.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAnimInstance.h"
#include "../HUD/OverHeadWidget.h"
#include "../Weapons/Weapon.h"
#include "../Components/CombatComponent.h"
#include "../Components/AttributeComponent.h"
#include "../PlayerController/MyPlayerController.h"
#include "../GameMode/MyGameMode.h"


AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	TurnRateGamepad = 50.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;	// 人物绕Z轴的旋转随着Controller绕Z转
	bUseControllerRotationRoll = false;

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

	// 经典数值，为什么这样设置？
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	CurrMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	MyController = Cast<AMyPlayerController>(Controller);

	if (OverHeadWidgetComponent)
	{
		UOverHeadWidget* OverHeadWidget = Cast<UOverHeadWidget>(OverHeadWidgetComponent->GetUserWidgetObject());
		if (OverHeadWidget)
		{
			OverHeadWidget->ShowPlayerNetRole(this, "Local");
		}
	}
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMyCharacter::ReceiveDamage);
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HideCharacterIfCameraClose();
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

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMyCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Test", IE_Pressed, this, &AMyCharacter::Test);
}

void AMyCharacter::Test()
{
	if (!IsWeaponEquipped()) Cout(this, "HH");
	if (!CombatComponent->EquippedWeapon) Cout(this, "FF");
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMyCharacter, OverlappingActor, COND_OwnerOnly);
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->MyCharacter = this;
		//CombatComponent->MyController = MyController;
	}
	if (AttributeComponent)
	{
		AttributeComponent->MyCharacter = this;
		//AttributeComponent->MyController = MyController;
	}
}

void AMyCharacter::MoveForward(float Value)
{
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
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyCharacter::JumpStart()
{
	Jump();
}

void AMyCharacter::JumpStop()
{
	StopJumping();
}

void AMyCharacter::CrouchStart()
{
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
	//UnCrouch();
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

void AMyCharacter::TargetStart()
{
	if (IsLocallyControlled() && CombatComponent)
	{
		SetIsAiming(true);
		CombatComponent->TargetStart();
	}
}

void AMyCharacter::TargetStop()
{
	if (IsLocallyControlled() && CombatComponent)
	{
		CombatComponent->TargetStop();
	}
}

void AMyCharacter::InteractStart()
{
	IInteractInterface* InteraceActor = OverlappingActor ? Cast<IInteractInterface>(OverlappingActor) : nullptr;
	if (InteraceActor)
	{
		InteraceActor->InteractStart(this);
	}
}

void AMyCharacter::InteractStop()
{
	IInteractInterface* InteraceActor = OverlappingActor ? Cast<IInteractInterface>(OverlappingActor) : nullptr;
	if (InteraceActor)
	{
		InteraceActor->InteractStop(this);
	}
}

void AMyCharacter::SetOverlappingActor(AActor* Actor)
{
	// 该函数仅会在Server调用， Weapon只会在Server中Overlap
	if (OverlappingActor)
	{
		if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
		{
			InteraceActor->SetInteractWidgetVisibility(false);
		}
	}
	OverlappingActor = Actor;// 该变量改变后会在客户端执行OnRep_OverlappingActor
	// OnRep_OverlappingActor仅会在Client进行，下面是为了让Server控制的角色有同样的逻辑
	if (IsLocallyControlled() && OverlappingActor)
	{
		if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
		{
			InteraceActor->SetInteractWidgetVisibility(true);
		}
	}
}

void AMyCharacter::Elim()
{
	
}

void AMyCharacter::OnRep_OverlappingActor(AActor* LastActor)
{
	if (IInteractInterface* InteraceActor = Cast<IInteractInterface>(OverlappingActor))
	{
		InteraceActor->SetInteractWidgetVisibility(true);
	}
	if (IInteractInterface* LastInteraceActor = Cast<IInteractInterface>(LastActor))
	{
		LastInteraceActor->SetInteractWidgetVisibility(false);
	}
}

void AMyCharacter::EquipWeaponStart()
{
	if (!CombatComponent)
	{
		return;
	}

	// Equip只在Server进行，通过复制变量通知Client
	if (HasAuthority())
	{
		CombatComponent->EquipWeaponStart(Cast<AWeapon>(OverlappingActor));
	}
	else
	{
		ServerEquipWeaponStart();
	}
}

void AMyCharacter::EquipWeaponStop()
{
}

void AMyCharacter::ServerEquipWeaponStart_Implementation()
{
	EquipWeaponStart();
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
	PlayHitReactMontage();
}

bool AMyCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bIsAiming;
}

bool AMyCharacter::IsAlive()
{
	return AttributeComponent && AttributeComponent->IsAlive();
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

FVector AMyCharacter::GetHitTarget() const
{
	return CombatComponent ? CombatComponent->MyHitTarget : FVector();
}

void AMyCharacter::SimProxiesTurn()
{
	if (!IsWeaponEquipped()) return;

	if (GetLocalRole() <= ENetRole::ROLE_SimulatedProxy)
	{
		// 去看视频里有关Turn的
	}
}

void AMyCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (AttributeComponent)
	{
		AttributeComponent->ReceiveDamage(DamageActor, Damage, DamageType, InstigatorController, DamageCauser);
	}
}
