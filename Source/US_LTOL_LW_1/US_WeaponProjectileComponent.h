// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "US_WeaponProjectileComponent.generated.h"

// Forward declarations
class AUS_BaseWeaponProjectile;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class AUS_Character;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class US_LTOL_LW_1_API UUS_WeaponProjectileComponent : public USceneComponent
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AUS_BaseWeaponProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext *WeaponMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction *ThrowAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	UAnimMontage *ThrowAnimation;

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	int32 MaxThrowCount = 3; // Maximum number of throws allowed

	UPROPERTY(Replicated)
	int32 CurrentThrowCount = 0; // Current number of throws used

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	float ReplenishDelay = 3.0f; // Time in seconds to wait before replenishing throws

	FTimerHandle ReplenishTimerHandle;

public:
	// Sets default values for this component's properties
	UUS_WeaponProjectileComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void Throw();
	UFUNCTION(Server, Reliable)
	void Throw_Server();

	UFUNCTION(NetMulticast, Reliable)
	void Throw_Client();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetProjectileClass(TSubclassOf<AUS_BaseWeaponProjectile> NewProjectileClass);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ResetThrowCount();

	UFUNCTION(BlueprintPure, Category = "Projectile")
	int32 GetRemainingThrows() const { return MaxThrowCount - CurrentThrowCount; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};
