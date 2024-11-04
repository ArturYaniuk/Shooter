// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/FlyingEnemy.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "./Characters/Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"


AFlyingEnemy::AFlyingEnemy() :
	AmmoType(EEnemyAmmoType::EEAT_MAX)
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AFlyingEnemy::OnHit);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 300.f;
	ProjectileMovementComponent->MaxSpeed = 300.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bIsHomingProjectile = true;
	ProjectileMovementComponent->HomingAccelerationMagnitude = 300.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

}

void AFlyingEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFlyingEnemy::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		auto EnemyOwner = Cast<AEnemy>(OtherActor);
		
		if(AmmoType != EEnemyAmmoType::EEAT_MAX)EnemyOwner->SetEnemyAmmo(AmmoType, 50);
		Destroy();
	}
}

void AFlyingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlyingEnemy::TakeParams(EEnemyAmmoType SpawnAmmoType)
{
	ProjectileMovementComponent->HomingTargetComponent = GetOwner()->GetRootComponent();
	AmmoType = SpawnAmmoType;
}



