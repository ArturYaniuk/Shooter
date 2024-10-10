// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Explosive.h"
#include "NiagaraComponent.h"
#include "../CharacterTypes.h"
#include "NiagaraComponent.h"
#include "items/Projectile.h"
#include "items/Weapons/AmmoType.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyState.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.0f);


	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate, bool bAlive = true);

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION()
	void SeePlayer(APawn* pawn);

	virtual void PostInitializeComponents() override;

	



private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	class UNiagaraSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	class USoundCue* ImpactSound;

	//Current Health of enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class AExplosive* ExplosiveSlot;

	//Name of the crit point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString CritBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	bool bCanHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	void ResetHitReactTimer();

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector SecondPatrolPoint;

	class AEnemyController* EnemyController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	//0: no stun chance, 1: 100% stun chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CombatRangeSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName DeathA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName DeathB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAlive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EProjectileType ProjectileType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AnimOffset, meta = (AllowPrivateAccess = "true"))
	bool bShoudUseAnimOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bSeePlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Awareness, meta = (AllowPrivateAccess = "true"))
	class UPawnSensingComponent* PawnSensor;

	UPROPERTY(BlueprintReadOnly, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PlayerPoint;
	//TODO: different attack animation section

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	EEnemyState EnemyState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


	FORCEINLINE FString GetCritBone() const { return CritBone; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE bool GetBShoudUseAnimOffset() { return bShoudUseAnimOffset; }

};
