// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/FlyingEnemy.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "./Characters/Enemy/Enemy.h"
#include "./Characters/Enemy/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"


AFlyingEnemy::AFlyingEnemy() :
	AmmoType(EAmmoType::EAT_MAX),
	Health(10.f),
	MaxHealth(10.f),
	bAlive(true),
	FlyingEnemyState(EFlyingEnemyState::EFES_MAX),
	AmmoSocketName("Default")
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFlyingEnemy::BeginPlay()
{
	Super::BeginPlay();
	SpawnPoint = this->GetActorLocation();
	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyReference)
	{
		EnemyOwner = Cast<AEnemy>(EnemyReference);
	}
	if (EnemyController)
	{
		EnemyController->RunBehaviorTree(BehaviorTree);
		EnemyController->GetBlackboardComponent()->SetValueAsObject("Target", EnemyReference);
	}
}


void AFlyingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFlyingEnemy::TakeParams(EAmmoType SpawnAmmoCarryType)
{
	const FString AmmoCaryTablePath{TEXT("/Script/Engine.DataTable'/Game/DataTable/AmmoCarryDataTable.AmmoCarryDataTable'")};

	UDataTable* AmmoCarryTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *AmmoCaryTablePath));
	if (AmmoCarryTableObject)
	{
		FAmmoCarryDataTable* AmmoCarryDataRow = nullptr;
		switch (SpawnAmmoCarryType)
		{
		case EAmmoType::EAT_MAX:
			AmmoCarryDataRow = AmmoCarryTableObject->FindRow<FAmmoCarryDataTable>(FName("Default"), TEXT(""));
			break;
		case EAmmoType::EAT_MainGun:
			AmmoCarryDataRow = AmmoCarryTableObject->FindRow<FAmmoCarryDataTable>(FName("MainGun"), TEXT(""));
			break;
		case EAmmoType::EAT_Rocket:
			AmmoCarryDataRow = AmmoCarryTableObject->FindRow<FAmmoCarryDataTable>(FName("Rocket"), TEXT(""));
			break;
		default:
			break;
		}
		if (AmmoCarryDataRow)
		{
			GetMesh()->SetSkeletalMesh(AmmoCarryDataRow->AmmoCarryMeshComponent);
			AmmoSocketName = AmmoCarryDataRow->AmmoSocketName;
		}
	}
	AmmoType = SpawnAmmoCarryType;
	ChangeState(EFlyingEnemyState::EFES_MoveToTarget);
}

void AFlyingEnemy::ReloadEnemy()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(1, 44, FColor::Green, TEXT("Start Reload"));

	if (AmmoType != EAmmoType::EAT_MAX && EnemyReference)EnemyReference->SetEnemyAmmo(AmmoType, 50);
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
		EnemyOwner->SetDeathFlyingEnemy(bAlive);
		Destroy();
	}

}

void AFlyingEnemy::ChangeState(EFlyingEnemyState NewState)
{
	FlyingEnemyState = NewState;
	OnFlyingEnemyStateChange.Broadcast(FlyingEnemyState);
}

