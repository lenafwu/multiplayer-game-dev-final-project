// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "US_MinionSpawner.generated.h"

UCLASS()
class US_LTOL_LW_1_API AUS_MinionSpawner : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> SpawnArea;
	UPROPERTY()
	FTimerHandle SpawnTimerHandle;

public:
	// Sets default values for this actor's properties
	AUS_MinionSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	TArray<TSubclassOf<class AUS_Minion>> SpawnableMinions;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	float SpawnDelay = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn System")
	int32 NumMinionsAtStart = 5;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	void Spawn();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
