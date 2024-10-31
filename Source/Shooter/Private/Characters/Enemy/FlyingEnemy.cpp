// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/FlyingEnemy.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AFlyingEnemy::AFlyingEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

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
	
	ProjectileMovementComponent->HomingTargetComponent = GetOwner()->GetRootComponent();
	
}

void AFlyingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


