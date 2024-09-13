// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Characters/Enemy/BulletHitInterface.h"
#include "NiagaraComponent.h"
#include "Engine/DataTable.h"
#include "Weapons/AmmoType.h"
#include "Projectile.generated.h"

USTRUCT()
struct FProjectileDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EProjectileType ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ProjectileMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldBounce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Bounciness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeSpan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactImpulse;
};


UCLASS()
class SHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void TakeSpawnProperties(EProjectileType ProjectileType);

	virtual void OnConstruction(const FTransform& Transform);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FireInDirection(const FVector& ShootDirection, EProjectileType Projectile);

private:
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite ,Category = Projectile, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ProjectileMeshComponent;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	EProjectileType ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float CollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float CollisionHalfHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float InitialSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	bool bShouldBounce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float Bounciness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float LifeSpan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties", meta = (AllowPrivateAccess = "true"))
	float ImpactImpulse;
};
