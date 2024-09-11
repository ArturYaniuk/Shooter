// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectile.h"
#include "./Characters/Enemy/Enemy.h"
#include "./Items/Weapons/Weapon.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));
    }

    if (!CollisionComponent)
    {
        // Use a sphere as a simple collision representation.
        CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SphereComponent"));
        // Set the sphere's collision profile name to "Projectile".
        CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
        // Event called when component hits something.
        CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
        // Set the sphere's collision radius.
        CollisionComponent->InitCapsuleSize(15.0f, 40.0f);
        // Set the root component to be the collision component.
        RootComponent = CollisionComponent;
    }

    if (!ProjectileMovementComponent)
    {
        // Use this component to drive this projectile's movement.
        ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
        ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
        ProjectileMovementComponent->InitialSpeed = 3000.0f;
        ProjectileMovementComponent->MaxSpeed = 3000.0f;
        ProjectileMovementComponent->bRotationFollowsVelocity = true;
        ProjectileMovementComponent->bShouldBounce = true;
        ProjectileMovementComponent->Bounciness = 0.3f;
        ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    }

    if (!ProjectileMeshComponent)
    {
        ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
        ProjectileMeshComponent->SetupAttachment(RootComponent);
    }

    // Delete the projectile after 3 seconds.
    InitialLifeSpan = 3.0f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::FireInDirection(const FVector& ShootDirection)
{
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != nullptr && OtherActor != this && OtherComponent != nullptr && OtherComponent->IsSimulatingPhysics())
	{

		OtherComponent->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * 100.0f, Hit.ImpactPoint);

	}

	if (OtherActor != nullptr && OtherActor != this && OtherComponent != nullptr)
	{

		FHitResult BeamHitResult;

		IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(OtherActor);
		if (BulletHitInterface)
		{
			BulletHitInterface->BulletHit_Implementation(Hit);
		}

		AEnemy* HitEnemy = Cast<AEnemy>(OtherActor);
		if (HitEnemy)
		{
		
		//	float HitDamage = Owner->GetEquippedWeapon()->GetDamage();
			float HitDamage = 1.0f;

			if (Hit.BoneName.ToString() == HitEnemy->GetCritBone())
			{
				HitDamage = 3.0f;
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("HEADSHOT"));
			}
			else
			{
				HitDamage = 1.0f;
			}

			UGameplayStatics::ApplyDamage(
				OtherActor,
				HitDamage,
				HitEnemy->GetController(),
				Owner,
				UDamageType::StaticClass());

		}
	


		else
		{
			// Spawn default particles
			if (ImpactParticles)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ImpactParticles,
					Hit.Location);
			}
		}
		
    }
    Destroy();
}

