// Fill out your copyright notice in the Description page of Project Settings.

#include "US_GameMode.h"
#include "US_PlayerController.h"
#include "US_PlayerState.h"
#include "US_Character.h"
#include "UObject/ConstructorHelpers.h"
#include "US_Minion.h"
#include "Kismet/GameplayStatics.h"

AUS_GameMode::AUS_GameMode()
{
    GameStateClass = AUS_GameState::StaticClass();

    PlayerStateClass = AUS_PlayerState::StaticClass();

    PlayerControllerClass = AUS_PlayerController::StaticClass();

    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_Character"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}

void AUS_GameMode::AlertMinions(AActor *AlertInstigator, const FVector &Location, const float Radius)
{
    TArray<AActor *> Minions;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUS_Minion::StaticClass(), Minions);
    for (const auto Minion : Minions)
    {
        if (AlertInstigator == Minion)
            continue;
        if (const auto Distance = FVector::Distance(AlertInstigator->GetActorLocation(), Minion->GetActorLocation()); Distance < Radius)
        {
            if (const auto MinionCharacter = Cast<AUS_Minion>(Minion))
            {
                MinionCharacter->GoToLocation(Location);
            }
        }
    }
}