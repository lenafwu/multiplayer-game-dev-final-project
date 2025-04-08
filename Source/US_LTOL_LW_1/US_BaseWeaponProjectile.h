// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "US_BaseWeaponProjectile.generated.h"

UCLASS()
class US_LTOL_LW_1_API AUS_BaseWeaponProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> SphereCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	float Damage = 10.f;

public:
	// Sets default values for this actor's properties
	AUS_BaseWeaponProjectile();

	FORCEINLINE USphereComponent *GetSphereCollision() const { return SphereCollision; }
	FORCEINLINE UStaticMeshComponent *GetMesh() const { return Mesh; }
	FORCEINLINE UProjectileMovementComponent *GetProjectileMovement() const { return ProjectileMovement; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
			   const FHitResult &Hit);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
