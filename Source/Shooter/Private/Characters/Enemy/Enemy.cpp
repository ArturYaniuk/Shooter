// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "./Characters/Enemy/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "NiagaraFunctionLibrary.h"
#include "Perception/PawnSensingComponent.h"
#include "./Characters/ShooterCharacter.h"

// Sets default values
AEnemy::AEnemy() :
	Health(1000.f),
	MaxHealth(1000.f),
	bCanHitReact(true),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(1.0f),
	bStunned(false),
	StunChance(0.5f),
	bInAttackRange(false),
	bAlive(true),
	bShoudUseAnimOffset(false),
	bSeePlayer(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	PawnSensor = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("Pawn Sensor"));

	PawnSensor->OnSeePawn.AddDynamic(this, &AEnemy::SeePlayer);
	PawnSensor->SetSensingUpdatesEnabled(true);

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeEndOverlap);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	if (ExplosiveSlot)
	{
		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
		ExplosiveSlot->AttachToComponent(GetMesh(), TransformRules, TEXT("Test_Socket"));
	}

	//Get the AI controller
	EnemyController = Cast<AEnemyController>(GetController());

	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);

	const FVector WorldSecondPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), SecondPatrolPoint);

	DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), WorldSecondPatrolPoint, 25.f, 12, FColor::Blue, true);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("SecondPatrolPoint"), WorldSecondPatrolPoint);
		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::Die()
{
	//const int32 Section = FMath::RandRange(0, 1); //uncomment for random death animation
	const int32 Section = 0;

	switch (Section)
	{
	case 0:
		PlayHitMontage(DeathA);
		DeathPose = EDeathPose::EDP_DeathA;
		break;

	case 1:
		PlayHitMontage(DeathB);
		DeathPose = EDeathPose::EDP_DeathB;
		break;
	default:
		break;

	}

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Death"), true);
	bAlive = false;
}


void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		bCanHitReact = false;

		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };

		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
	}
}



void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}
	
}

void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShooterCharacter)
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate, bool isAlive)
{
	if (isAlive)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && AttackMontage)
		{
			AnimInstance->Montage_Play(AttackMontage);
			AnimInstance->Montage_JumpToSection(Section, AttackMontage);
		}
	}
	else return;
}

void AEnemy::Attack()
{
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			//(GetWorld(), EquippedWeapon->GetMuzzleFash(), SocketTransform);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlash, SocketTransform.GetLocation(), GetViewRotation());
		}


		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), GetViewRotation(), SpawnParams);

		if (Projectile) Projectile->FireInDirection(GetViewRotation().Vector(), ProjectileType, 1.0f, 1.5f);

	}
}

void AEnemy::SeePlayer(APawn* pawn)
{
	if (pawn == nullptr) return;

	auto ShooterCharacter = Cast<AShooterCharacter>(pawn);

	if (ShooterCharacter)
	{
		bSeePlayer = true;
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), ShooterCharacter);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAgroRange"), true);
	}
}



void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, HitResult.Location, HitResult.ImpactNormal.Rotation());
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	if (Health - DamageAmount <- 0.f)
	{
		Health = 0.f;
		if (bAlive)		Die();
	}
	else
	{
		Health -= DamageAmount;
		const float Stunned = FMath::FRandRange(0.f, 1.f);

		if (Stunned <= StunChance)
		{
			PlayHitMontage(FName("HitReactFront"));
			SetStunned(true);
		}
	
	}
	return DamageAmount;
}

