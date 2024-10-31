// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "FlyingEnemy.generated.h"

UCLASS()
class SHOOTER_API AFlyingEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AFlyingEnemy();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;


private:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;
};
