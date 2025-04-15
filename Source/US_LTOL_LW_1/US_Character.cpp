// Fill out your copyright notice in the Description page of Project Settings.

#include "US_Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
// Inputs
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "US_CharacterStats.h"
#include "Engine/DataTable.h"

// Interactable
#include "US_Interactable.h"
#include "Kismet/KismetSystemLibrary.h"

// noise
#include "Components/PawnNoiseEmitterComponent.h"

#include "US_WeaponProjectileComponent.h"

// Sets default values
AUS_Character::AUS_Character()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
	NoiseEmitter->NoiseLifetime = 0.01f;

	Weapon = CreateDefaultSubobject<UUS_WeaponProjectileComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(RootComponent);
	Weapon->SetRelativeLocation(FVector(120.f, 70.f, 0.f));
	Weapon->SetIsReplicated(true);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCapsuleComponent()->InitCapsuleSize(60.f, 96.0f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -91.f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/KayKit/Characters/rogue"));
	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

// Called when the game starts or when spawned
void AUS_Character::BeginPlay()
{
	Super::BeginPlay();
	// checks that the Controller is a PlayerController by means of a Cast template.
	if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UpdateCharacterStats(1);
}

// Called every frame
void AUS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() != ROLE_Authority)
		return;
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(this);
	auto SphereRadius = 50.f;
	auto StartLocation = GetActorLocation() + GetActorForwardVector() * 150.f;
	auto EndLocation = StartLocation + GetActorForwardVector() * 500.f;
	auto IsHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
		SphereRadius,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		TArray<AActor *>(),
		EDrawDebugTrace::ForOneFrame,
		HitResult,
		true);

	if (IsHit && HitResult.GetActor()->GetClass()->ImplementsInterface(UUS_Interactable::StaticClass()))
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, SphereRadius, 12, FColor::Magenta, false, 1.f);
		InteractableActor = HitResult.GetActor();
	}
	else
	{
		InteractableActor = nullptr;
	}

	// make noise when sprinting, higher level is quieter
	if (GetCharacterMovement()->MaxWalkSpeed == GetCharacterStats()->SprintSpeed)
	{
		auto Noise = 1.f;
		if (GetCharacterStats() && GetCharacterStats()->StealthMultiplier)
		{
			Noise = Noise / GetCharacterStats()->StealthMultiplier;
		}
		NoiseEmitter->MakeNoise(this, Noise, GetActorLocation());
	}
}

// Called to bind functionality to input
void AUS_Character::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// bind these actions to a corresponding method implementation
	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUS_Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUS_Character::Look);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,
										   &AUS_Character::Interact);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this,
										   &AUS_Character::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this,
										   &AUS_Character::SprintEnd);
	}
}

void AUS_Character::Move(const FInputActionValue &Value)
{
	const auto MovementVector = Value.Get<FVector2D>();
	// GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Yellow, FString::Printf(TEXT("MovementVector: %s"), *MovementVector.ToString()));
	if (Controller != nullptr)
	{
		const auto Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const auto ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const auto RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AUS_Character::Look(const FInputActionValue &Value)
{
	const auto LookAxisVector = Value.Get<FVector2D>();
	// GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("LookAxisVector: %s"), *LookAxisVector.ToString()));
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUS_Character::SprintStart(const FInputActionValue &Value)
{
	// GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Blue, TEXT("SprintStart"));
	// // GetCharacterMovement()->MaxWalkSpeed = 3000.f;
	// if (GetCharacterStats())
	// {
	// 	GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->SprintSpeed;
	// }

	SprintStart_Server();
}

void AUS_Character::SprintEnd(const FInputActionValue &Value)
{
	// GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Blue, TEXT("SprintEnd"));
	// // GetCharacterMovement()->MaxWalkSpeed = 500.f;
	// if (GetCharacterStats())
	// {
	// 	GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->WalkSpeed;
	// }

	SprintEnd_Server();
}

void AUS_Character::Interact(const FInputActionValue &Value)
{
	// GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, TEXT("Interact"));

	Interact_Server();
}

void AUS_Character::Interact_Server_Implementation()
{
	if (InteractableActor)
	{
		IUS_Interactable::Execute_Interact(InteractableActor, this);
	}
}

void AUS_Character::UpdateCharacterStats(int32 CharacterLevel)
{
	auto IsSprinting = false;
	if (GetCharacterStats())
	{
		IsSprinting = GetCharacterMovement()->MaxWalkSpeed == GetCharacterStats()->SprintSpeed;
	}

	if (CharacterDataTable)
	{
		TArray<FUS_CharacterStats *> CharacterStatsRows;
		CharacterDataTable->GetAllRows<FUS_CharacterStats>(TEXT("US_Character"), CharacterStatsRows);
		if (CharacterStatsRows.Num() > 0)
		{
			const auto NewCharacterLevel = FMath::Clamp(CharacterLevel, 1, CharacterStatsRows.Num());
			CharacterStats = CharacterStatsRows[NewCharacterLevel - 1];
			GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->WalkSpeed;
			if (IsSprinting)
			{
				SprintStart_Server();
			}
		}
	}
}

void AUS_Character::SprintStart_Server_Implementation()
{
	// if (GetCharacterStats())
	// {
	// 	GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->SprintSpeed;
	// }
	SprintStart_Client();
}
void AUS_Character::SprintEnd_Server_Implementation()
{
	// if (GetCharacterStats())
	// {
	// 	GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->WalkSpeed;
	// }
	SprintEnd_Client();
}

void AUS_Character::SprintStart_Client_Implementation()
{
	if (GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->SprintSpeed;
	}
}
void AUS_Character::SprintEnd_Client_Implementation()
{
	if (GetCharacterStats())
	{
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterStats()->WalkSpeed;
	}
}
