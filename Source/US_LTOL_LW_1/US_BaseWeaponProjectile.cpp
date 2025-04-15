// Fill out your copyright notice in the Description page of Project Settings.

#include "US_BaseWeaponProjectile.h"
#include "US_Character.h"
#include "US_CharacterStats.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AUS_BaseWeaponProjectile::AUS_BaseWeaponProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// collision
	SphereCollision = CreateDefaultSubobject<USphereComponent>("Collision");
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetSphereRadius(10.0f);
	SphereCollision->BodyInstance.SetCollisionProfileName("BlockAll");
	SphereCollision->OnComponentHit.AddDynamic(this, &AUS_BaseWeaponProjectile::OnHit);
	RootComponent = SphereCollision;

	// mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Mesh->SetRelativeLocation(FVector(-40.f, 0.f, 0.f));
	Mesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMesh(TEXT("/Game/Models/Axe"));

	if (StaticMesh.Succeeded())
	{
		GetMesh()->SetStaticMesh(StaticMesh.Object);
	}

	// projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->UpdatedComponent = SphereCollision;
	ProjectileMovement->ProjectileGravityScale = 0;
	ProjectileMovement->InitialSpeed = 3000;
	ProjectileMovement->MaxSpeed = 3000;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AUS_BaseWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AUS_BaseWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUS_BaseWeaponProjectile::OnHit(UPrimitiveComponent *HitComponent, AActor *OtherActor,
									 UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
	auto ComputedDamage = Damage;
	if (const auto Character = Cast<AUS_Character>(GetInstigator()))
	{
		ComputedDamage *= Character->GetCharacterStats()->DamageMultiplier;
	}

	// Add debug message
	// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
	// 								 FString::Printf(TEXT("Projectile hit: %s with damage: %.1f"),
	// 												 *OtherActor->GetName(), ComputedDamage));

	if (OtherActor && OtherActor != this)
	{
		const FDamageEvent DamageEvent(UDamageType::StaticClass());
		const float ActualDamage = OtherActor->TakeDamage(ComputedDamage, DamageEvent, GetInstigatorController(), this);

		// Add debug message for actual damage applied
		// GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
		// 								 FString::Printf(TEXT("Applied damage: %.1f"), ActualDamage));
	}
	Destroy();
}
