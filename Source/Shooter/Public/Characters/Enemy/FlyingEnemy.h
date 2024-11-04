// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "items/Weapons/AmmoType.h"
#include "FlyingEnemy.generated.h"

UCLASS()
class SHOOTER_API AFlyingEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AFlyingEnemy();
	virtual void Tick(float DeltaTime) override;

	void TakeParams(EEnemyAmmoType AmmoType);


protected:
	virtual void BeginPlay() override;


private:

	UPROPERTY(VisibleDefaultsOnly, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AmmoCarry, meta = (AllowPrivateAccess = "true"))
	EEnemyAmmoType AmmoType;
};
