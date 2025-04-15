// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "US_PlayerState.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnXpChanged, int32, NewXp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLevelUp, int32, NewLevelXp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);

UCLASS()
class US_LTOL_LW_1_API AUS_PlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	// xp and level
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_Xp", Category = "Experience")
	int Xp = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_CharacterLevelUp", Category = "Experience")
	int CharacterLevel = 1;

	// health
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_Health", Category = "Health")
	float Health = 100.0f;

	UPROPERTY()
	FTimerHandle RespawnTimerHandle;

	UFUNCTION()
	void OnRep_Xp(int32 OldValue) const;
	UFUNCTION()
	void OnRep_CharacterLevelUp(int32 OldValue) const;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnXpChanged OnXpChanged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterLevelUp OnCharacterLevelUp;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float RespawnDelay = 3.0f; // Time before respawn

public:
	UFUNCTION(BlueprintCallable, Category = "Experience")
	void AddXp(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION()
	void Die();

	UFUNCTION()
	void Respawn();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};