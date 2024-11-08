// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "items/Weapons/AmmoType.h"
#include "Engine/DataTable.h"
#include "EnemyState.h"
#include "items/Ammo.h"
#include "FlyingEnemy.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlyingEnemyStateChange, EFlyingEnemyState, FlyingEnemyState);

USTRUCT()
struct FAmmoCarryDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* AmmoCarryMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AmmoSocketName;

};

UCLASS()
class SHOOTER_API AFlyingEnemy : public ACharacter
{
	GENERATED_BODY()

public:

	AFlyingEnemy();
	virtual void Tick(float DeltaTime) override;

	void TakeParams(EAmmoType AmmoType);

	UFUNCTION(BlueprintCallable)
	void ReloadEnemy();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	void ChangeState(EFlyingEnemyState NewState);



protected:

	virtual void BeginPlay() override;



private:

	UPROPERTY(VisibleDefaultsOnly, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bAlive;

	FVector SpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	EFlyingEnemyState FlyingEnemyState;

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FOnFlyingEnemyStateChange OnFlyingEnemyStateChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	FName AmmoSocketName;

public:
	FORCEINLINE FName GetSocketName() { return AmmoSocketName; }
};
