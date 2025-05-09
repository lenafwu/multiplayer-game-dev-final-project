// Fill out your copyright notice in the Description page of Project Settings.

#include "US_WeaponProjectileComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "US_BaseWeaponProjectile.h"
#include "US_Character.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UUS_WeaponProjectileComponent::UUS_WeaponProjectileComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// the base projectile that will be spawned when the throw action is triggered
	ProjectileClass = AUS_BaseWeaponProjectile::StaticClass();
}

// Called when the game starts
void UUS_WeaponProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get our custom character class
	const AUS_Character *Character = Cast<AUS_Character>(GetOwner());
	if (!Character)
		return;

	// Setup input for both server and client using our custom player controller
	if (APlayerController *PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		// Add the mapping context
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
		}

		// Bind the input action
		if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &UUS_WeaponProjectileComponent::Throw);
		}
	}
}

// Called every frame
void UUS_WeaponProjectileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUS_WeaponProjectileComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UUS_WeaponProjectileComponent, CurrentThrowCount);
}

void UUS_WeaponProjectileComponent::Throw()
{
	if (CurrentThrowCount >= MaxThrowCount)
	{
		return;
	}

	Throw_Server();
}

void UUS_WeaponProjectileComponent::Throw_Server_Implementation()
{
	if (CurrentThrowCount >= MaxThrowCount)
		return;

	if (ProjectileClass)
	{
		CurrentThrowCount++; // Increment throw counter

		// Start replenish timer when all throws are used
		if (CurrentThrowCount >= MaxThrowCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				ReplenishTimerHandle,
				this,
				&UUS_WeaponProjectileComponent::ResetThrowCount,
				ReplenishDelay,
				false);
		}

		Throw_Client();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
											   {
			const auto Character = Cast<AUS_Character>(GetOwner());
			const auto ProjectileSpawnLocation = GetComponentLocation();
			const auto ProjectileSpawnRotation = GetComponentRotation();
			auto ProjectileSpawnParams = FActorSpawnParameters();
			ProjectileSpawnParams.Owner = GetOwner();
			ProjectileSpawnParams.Instigator = Character;
			GetWorld()->SpawnActor<AUS_BaseWeaponProjectile>(
				ProjectileClass, 
				ProjectileSpawnLocation, 
				ProjectileSpawnRotation, 
				ProjectileSpawnParams
			); }, .4f, false);
	}
}

void UUS_WeaponProjectileComponent::SetProjectileClass(TSubclassOf<AUS_BaseWeaponProjectile> NewProjectileClass)
{
	ProjectileClass = NewProjectileClass;
}

void UUS_WeaponProjectileComponent::Throw_Client_Implementation()
{
	const auto Character = Cast<AUS_Character>(GetOwner());
	if (ThrowAnimation != nullptr)
	{
		if (const auto AnimInstance = Character->GetMesh()->GetAnimInstance(); AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(ThrowAnimation, 1.f);
		}
	}
}

void UUS_WeaponProjectileComponent::ResetThrowCount()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		CurrentThrowCount = 0;
	}
}