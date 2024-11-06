// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/FlyingEnemy.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "./Characters/Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"


AFlyingEnemy::AFlyingEnemy() :
	AmmoType(EEnemyAmmoType::EEAT_MAX),
	Health(10.f),
	MaxHealth(10.f),
	bAlive(true),
	FlyingEnemyState(EFlyingEnemyState::EFES_MAX)
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
	SpawnPoint = this->GetActorLocation();
}


void AFlyingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlyingEnemy::TakeParams(EEnemyAmmoType SpawnAmmoType)
{
	ProjectileMovementComponent->HomingTargetComponent = GetOwner()->GetRootComponent();
	AmmoType = SpawnAmmoType;
	ChangeState(EFlyingEnemyState::EFES_MoveToTarget);
}

void AFlyingEnemy::ReloadEnemy()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(1, 44, FColor::Green, TEXT("Start Reload"));
	auto EnemyOwner = Cast<AEnemy>(GetOwner());
	if (AmmoType != EEnemyAmmoType::EEAT_MAX)EnemyOwner->SetEnemyAmmo(AmmoType, 50);
	ChangeState(EFlyingEnemyState::EFES_MoveHome);

}

float AFlyingEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount < -0.f)
	{
		Health = 0.f;

		if (bAlive) Die();
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AFlyingEnemy::Die()
{
	if (bAlive)
	{
		ChangeState(EFlyingEnemyState::EFES_Death);
		bAlive = false;
		Destroy();
	}

}

void AFlyingEnemy::ChangeState(EFlyingEnemyState NewState)
{
	FlyingEnemyState = NewState;
	OnFlyingEnemyStateChange.Broadcast(FlyingEnemyState);
}
