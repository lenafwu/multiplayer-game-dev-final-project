// Fill out your copyright notice in the Description page of Project Settings.

#include "US_PlayerState.h"
#include "US_Character.h"
#include "US_CharacterStats.h"
#include "US_WeaponProjectileComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AUS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(AUS_PlayerState, Xp, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AUS_PlayerState, CharacterLevel, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(AUS_PlayerState, Health, COND_OwnerOnly);
}

void AUS_PlayerState::AddXp(const int32 Value)
{
    Xp += Value;
    OnXpChanged.Broadcast(Xp);
    // GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Yellow, FString::Printf(TEXT("Total Xp: %d"), Value));
    if (const auto Character = Cast<AUS_Character>(GetPawn()))
    {
        if (Character->GetCharacterStats()->NextLevelXp < Xp)
        {
            // GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Red, TEXT("Level Up!"));
            CharacterLevel++;
            Character->UpdateCharacterStats(CharacterLevel);
            OnCharacterLevelUp.Broadcast(CharacterLevel);
        }
    }
}

void AUS_PlayerState::OnRep_Xp(int32 OldValue) const
{
    OnXpChanged.Broadcast(Xp);
}

void AUS_PlayerState::OnRep_CharacterLevelUp(int32 OldValue) const
{
    OnCharacterLevelUp.Broadcast(CharacterLevel);
}

void AUS_PlayerState::ApplyDamage(float Amount)
{
    if (GetLocalRole() != ROLE_Authority)
        return;

    Health -= Amount;
    OnHealthChanged.Broadcast(Health);

    if (Health <= 0)
    {
        Die();
    }
}

void AUS_PlayerState::Die()
{
    // Make sure health is 0
    Health = 0;
    OnHealthChanged.Broadcast(Health);

    // Get the character and hide/disable it
    if (auto Character = Cast<AUS_Character>(GetPawn()))
    {
        Character->SetActorHiddenInGame(true);
        Character->SetActorEnableCollision(false);
        // Optionally disable input
        if (auto PC = Cast<APlayerController>(Character->GetController()))
        {
            PC->DisableInput(PC);
        }
    }

    // Start respawn timer
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AUS_PlayerState::Respawn,
        RespawnDelay,
        false);
}

void AUS_PlayerState::Respawn()
{
    // Reset health
    Health = 100.0f;
    OnHealthChanged.Broadcast(Health);

    if (AGameModeBase *GM = Cast<AGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        if (AController *Controller = Cast<AController>(GetOwner()))
        {
            // Find a random PlayerStart
            AActor *StartSpot = GM->FindPlayerStart(Controller);

            // Respawn at the chosen PlayerStart
            APawn *NewPawn = GM->SpawnDefaultPawnFor(Controller, StartSpot);

            if (NewPawn)
            {
                // Possess the new pawn and enable input
                Controller->Possess(NewPawn);
                if (auto PC = Cast<APlayerController>(Controller))
                {
                    PC->EnableInput(PC);
                }

                // The weapon component will initialize itself in BeginPlay
                // No need to manually reinitialize it here
            }
        }
    }
}

void AUS_PlayerState::OnRep_Health()
{
    OnHealthChanged.Broadcast(Health);
}
