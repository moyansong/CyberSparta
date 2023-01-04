// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAnimInstance.h"
#include "../CyberSparta.h"
#include "../HUD/OverHeadWidget.h"
#include "../Weapons/Weapon.h"
#include "../Components/CombatComponent.h"
#include "../Components/BuffComponent.h"
#include "../Components/AttributeComponent.h"
#include "../Components/LagCompensationComponent.h"
#include "../PlayerController/MyPlayerController.h"
#include "../GameMode/MyGameMode.h"
#include "../PlayerStates/MyPlayerState.h"
#include "../GameStates/MyGameState.h"
#include "../Game/TeamPlayerStart.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	TurnRateGamepad = 50.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;	
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
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HideCharacterIfCameraClose();
	Initialized();
}

void AMyCharacter::Initialized()
{
	// 让其只发生一次
	if (!MyController)
	{
		MyController = Cast<AMyPlayerController>(Controller);
		if (MyController) OnControllerInitialized();
	}
	if (!MyPlayerState)
	{
		MyPlayerState = GetPlayerState<AMyPlayerState>();
		if (MyPlayerState) OnPlayerStateInitialized();
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

void AMyCharacter::OnControllerInitialized()
{
	if (MyController)
	{
		MyController->HigPingDelegate.AddDynamic(this, &AMyCharacter::OnPingIsTooHigh);
	}
	if (AttributeComponent)
	{
		AttributeComponent->SetHUDHealth();
		AttributeComponent->SetHUDShield();
	}
	if (CombatComponent)
	{
		CombatComponent->SetHUDWeapon();
	}
}

void AMyCharacter::OnPlayerStateInitialized()
{
	AMyGameState* MyGameState = Cast<AMyGameState>(UGameplayStatics::GetGameState(this));
	if (MyPlayerState && MyGameState && MyGameState->TopScoringPlayers.Contains(MyPlayerState))
	{
		MulticastGainTheLead();
	}
	SetTeam(MyPlayerState->GetTeam());
	SetSpawnLocation();
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
		UAnimInstance* MyAnimInstance = GetMesh()->GetAnimInstance();
		if (MyAnimInstance)
		{
			MyAnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnMontageEnded);
		}
	}
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
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AMyCharacter::RunStart);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AMyCharacter::RunStop);

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
	if (GetCombatState() == ECombatState::ECS_Idle)
	{
		Cout(this, "Idle");
	}
	else if (GetCombatState() == ECombatState::ECS_Firing)
	{
		Cout(this, "Firing");
	}
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

bool AMyCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->bIsAiming;
}

void AMyCharacter::SetMaxWalkSpeed(bool bIsRunning)
{
	if (!GetCharacterMovement()) return;

	if (bIsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = AimingJogSpeed;
	}
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

FVector AMyCharacter::GetHitTarget() const
{
	return CombatComponent ? CombatComponent->MyHitTarget : FVector();
}

void AMyCharacter::FireStart()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->FireStart();
	}
}

void AMyCharacter::FireStop()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->FireStop();
	}
}

void AMyCharacter::TargetStart()
{
	if (bDisableGameplay) return;
	/*if (IsLocallyControlled() && CombatComponent)
	{
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
	if (bDisableGameplay) return;
}

void AMyCharacter::ThrowWeaponStop()
{
	if (bDisableGameplay) return;
	if (IsWeaponEquipped())
	{
		CombatComponent->ThrowWeapon();
	}
}

void AMyCharacter::RunStart()
{
	UnCrouch();
	if (CombatComponent)
	{
		CombatComponent->SetIsAiming(false);
	}
	SetMaxWalkSpeed(true);
	if (!HasAuthority())
	{
		ServerRunStart();
	}
}

void AMyCharacter::RunStop()
{
	if (CombatComponent)
	{
		CombatComponent->SetIsAiming(true);
	}
	SetMaxWalkSpeed(false);
	if (!HasAuthority())
	{
		ServerRunStop();
	}
}

void AMyCharacter::ServerRunStart_Implementation()
{
	RunStart();
}

void AMyCharacter::ServerRunStop_Implementation()
{
	RunStop();
}

void AMyCharacter::EquipWeapon(float Value)
{
	if (bDisableGameplay || FMath::IsWithin(Value, -0.6f, 0.6f)) return;
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(Value > 0.f ? 1 : (Value < 0.f ? -1 : 0)); 
	}
}

AWeapon* AMyCharacter::GetEquippedWeapon() const
{
	return CombatComponent ? CombatComponent->EquippedWeapon : nullptr;
}

bool AMyCharacter::IsWeaponEquipped() const
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

void AMyCharacter::SimulateHit(const FRotator& HitDirection, const FVector& HitLocation)
{
	/*float HitDirectionYaw = HitDirection.Yaw - GetActorRotation().Yaw;
	FName Section("HitByFwd");
	if (HitDirectionYaw >= 45.f && HitDirectionYaw < 135.f) Section = FName("HitByLt");
	else if (HitDirectionYaw >= -135.f && HitDirectionYaw  < -45.f) Section = FName("HitByRt");
	else if (HitDirectionYaw >= -45.f && HitDirectionYaw < 45.f) Section = FName("HitByBwd");*/

	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered(HitLocation.X, HitLocation.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Theta总是>0
	double Theta = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

	// ue是左手系，叉乘遵循左手定则
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	Theta *= CrossProduct.Z < 0 ? -1.f : 1.f;

	FName Section("HitByBwd");
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("HitByFwd");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("HitByLt");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("HitByRt");
	}
	PlayHitReactMontage(Section);

	/*Cout(this, Section.ToString());
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Red, 10.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Green, 10.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 60.f, 5.f, FColor::Blue, 10.f);*/
}

void AMyCharacter::MulticastHit_Implementation(const FRotator& HitDirection, const FVector_NetQuantize& HitLocation)
{
	SimulateHit(HitDirection, HitLocation);
}

void AMyCharacter::PlayHitReactMontage(FName SectionName)
{
	if (!IsWeaponEquipped() || !GetMesh() || !GetCharacterMovement() || GetCharacterMovement()->Velocity.Size() > 10.f) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* HitReactMontage = GetEquippedWeapon()->HitReactMontage;
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
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

float AMyCharacter::GetHealth() const
{
	return AttributeComponent ? AttributeComponent->GetHealth() : 0.f;
}

bool AMyCharacter::IsAlive() const
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

void AMyCharacter::Eliminate()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	SimulateEliminate();
}

void AMyCharacter::SimulateEliminate()
{
	if (AttributeComponent && AttributeComponent->EliminateMontage)
	{
		PlayAnimMontage(AttributeComponent->EliminateMontage);
	}
}

void AMyCharacter::KillReward()
{
	if (CombatComponent)
	{
		CombatComponent->KillReward();
	}
}

void AMyCharacter::OnPingIsTooHigh(float Ping)
{
	//bShouldMulticastEffect = Ping > 200.f;
}

void AMyCharacter::LeaveGame()
{
	ServerLeaveGame();
	if (IsLocallyControlled())
	{
		OnLeftGame.Broadcast();// 可能延迟一会在广播，先让Server处理一些数据
	}
}

void AMyCharacter::ServerLeaveGame_Implementation()
{
	AMyGameMode* MyGameMode = GetWorld()->GetAuthGameMode<AMyGameMode>();
	MyPlayerState = MyPlayerState ? MyPlayerState : GetPlayerState<AMyPlayerState>();
	if (MyGameMode && MyPlayerState)
	{
		MyGameMode->PlayerLeftGame(MyPlayerState);
	}
}

void AMyCharacter::MulticastGainTheLead_Implementation()
{
	if (!CrownSystem) return;
	if (!CrownComponent)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		); 
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void AMyCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void AMyCharacter::SetTeam(ETeam Team)
{
	if (!GetMesh()) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		if (OriginalMaterial) GetMesh()->SetMaterial(0, OriginalMaterial);
		break;
	case ETeam::ET_BlueTeam:
		if (BlueTeamMaterial)GetMesh()->SetMaterial(0, BlueTeamMaterial);
		break;
	case ETeam::ET_RedTeam:
		if (RedTeamMaterial)GetMesh()->SetMaterial(0, RedTeamMaterial);
		break;
	}
}

ETeam AMyCharacter::GetTeam() const
{
	// MyPlayerState = MyPlayerState ? MyPlayerState : GetPlayerState<AMyPlayerState>();
	// 不进行上一行的原因是，有时过早的调用GetTeam会导致一些bug
	return MyPlayerState ? MyPlayerState->GetTeam() : ETeam::ET_NoTeam;
}

void AMyCharacter::SetSpawnLocation()
{
	// 为什么这事不在TeamGameMode里做是因为要确保人物有了Team后在设置位置，Team是在PlayerState里设置的
	ETeam MyTeam = GetTeam();
	if (HasAuthority() && MyTeam != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == MyTeam)
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (!TeamPlayerStarts.IsEmpty())
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
	}
}

void AMyCharacter::SetAnimationClass()
{
	if (!GetMesh()) return;

	if (GetEquippedWeapon() && GetEquippedWeapon()->CharacterAnimationClass)
	{
		GetMesh()->SetAnimInstanceClass(GetEquippedWeapon()->CharacterAnimationClass);
	}
	else
	{
		GetMesh()->SetAnimInstanceClass(DefaultAnimationClass);
	}
}

void AMyCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CombatComponent) CombatComponent->OnMontageEnded(Montage, bInterrupted);
}

