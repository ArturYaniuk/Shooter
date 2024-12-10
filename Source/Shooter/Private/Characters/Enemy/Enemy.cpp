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
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "./Characters/ShooterCharacter.h"
#include "./Characters/Enemy/FlyingEnemy.h"
#include "NavigationData.h"
#include "Items/Weapons/AmmoType.h"
#include "ActorComponents/HealthComponent.h"
#include "ActorComponents/AttackComponent.h"

// Sets default values
AEnemy::AEnemy() :
	bCanHitReact(true),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(1.0f),
	bStunned(false),
	StunChance(0.01f),
	bInAttackRange(false),
	bShoudUseAnimOffset(false),
	bSeePlayer(false),
	EnemyState(EEnemyState::EES_Passive),
	PreviousState(EnemyState),
	DefaultMainGunAmmo(5),
	DefaultRocketGunAmmo(8),
	bAmmoCarryAlive(false),
	SpawnAmmoCarryMin(10),
	SpawnAmmoCarryMax(20)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());



	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("Attack Component"));

	SetupStimulusSource();
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitializeAmmoMap();

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
	if (EnemyController) OnEnemyStateChange.Broadcast(EnemyState);


	Target = Cast<AShooterCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

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
	SetState(EEnemyState::EES_Death);
	EnemyController->StopMovement();
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
	if (EnemyController)
	{
		EnemyController->StopMovement();
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("bStunned"), bStunned);
		const float StunTime{ FMath::FRandRange(HitReactTimeMin, HitReactTimeMax) };

		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetStun, StunTime);
	}
}

void AEnemy::ResetStun()
{
	bStunned = false; 
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("bStunned"), bStunned);
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
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
			SetState(EEnemyState::EES_Attacking);
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
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
		}
		SetState(EEnemyState::EES_MoveToTarget);
		if (Target)	EnemyController->MoveToActor(Target, 500.f);
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
	if (AmmoMap[EAmmoType::EAT_MainGun] > 0)
	{

		DecrementAmmo(EAmmoType::EAT_MainGun);

		AttackComponent->SpawnProjectile(
			GetMesh()->GetSocketByName("BarrelSocket"),
			GetMesh(),
			MuzzleFlash,
			UKismetMathLibrary::GetDirectionUnitVector(GetMesh()->GetSocketLocation("BarrelSocket"), Target->GetActorLocation()),
			ProjectileType,
			1.0f, 1.5f);
	}
	else
	{
		TakeAmmo();
	}
}

void AEnemy::SetState(EEnemyState newState)
{
	if (HealthComponent->IsAlive())
	{
		EnemyState = newState;
	}
	else
	{
		EnemyState = EEnemyState::EES_Death;
	}
	OnEnemyStateChange.Broadcast(EnemyState);
}

void AEnemy::TakeAmmo()
{
	SpawnCarry();
}

AFlyingEnemy* AEnemy::SpawnCarry()
{
	if (AmmoCarry && !bAmmoCarryAlive) 
	{
		FVector position(SecondPatrolPoint.X, SecondPatrolPoint.Y, 800);
		FRotator rotator(GetActorRotation());

		

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = GetInstigator();

		AFlyingEnemy* FlyingEnemy = GetWorld()->SpawnActor<AFlyingEnemy>(AmmoCarry, position, rotator, SpawnInfo);
		
		FlyingEnemy->TakeParams(EAmmoType::EAT_MainGun);
		FlyingEnemy->SetTarget(this);
		StartSpawnAmmoCarryTimer();
		
		SpawnAmmo(EAmmoType::EAT_MainGun, FlyingEnemy);
	}
	return nullptr;
}

AAmmo* AEnemy::SpawnAmmo(EAmmoType LocalAmmoType , AFlyingEnemy* ParentAmmoCarry)
{
	FVector position(ParentAmmoCarry->GetMesh()->GetSocketLocation(ParentAmmoCarry->GetSocketName()));
	FRotator rotator(GetActorRotation());

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = GetInstigator();

	Ammo = GetWorld()->SpawnActor<AAmmo>(CarriedAmmo, position, rotator, SpawnInfo);
	Ammo->SetAmmoType(LocalAmmoType);

	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	Ammo->AttachToComponent(ParentAmmoCarry->GetMesh(), TransformRules, ParentAmmoCarry->GetSocketName());


	return nullptr;
}

void AEnemy::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_MainGun, DefaultMainGunAmmo);
	AmmoMap.Add(EAmmoType::EAT_Rocket, DefaultRocketGunAmmo);
}

void AEnemy::DecrementAmmo(EAmmoType AmmoType)
{
	if (AmmoMap[AmmoType] - 1 <= 0) AmmoMap[AmmoType] = 0;
	else --AmmoMap[AmmoType];
}

void AEnemy::StartSpawnAmmoCarryTimer()
{
	bAmmoCarryAlive = true;
	const float SpawnAmmoCarryDelay{ FMath::FRandRange(SpawnAmmoCarryMin, SpawnAmmoCarryMax) };
	GetWorldTimerManager().SetTimer(SpawnAmmoCarryTimer, this, &AEnemy::ResetSpawnAmmoCarryTimer, SpawnAmmoCarryDelay);
}

void AEnemy::ResetSpawnAmmoCarryTimer()
{
	bAmmoCarryAlive = false;
}

void AEnemy::SetEnemyAmmo(EAmmoType AmmoType, float Amount)
{
	AmmoMap[AmmoType] += Amount; 
	Ammo->Destroy();
}

void AEnemy::SetDeathFlyingEnemy(bool newbAmmoCarryAlive)
{
	bAmmoCarryAlive = newbAmmoCarryAlive;
	
	if (Ammo)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		Ammo->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		Ammo->SetItemState(EItemState::EIS_Falling);
	}
}



void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::SetupStimulusSource()
{
	StimulusSourse = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimuls"));
	if (StimulusSourse)
	{
		StimulusSourse->RegisterForSense(TSubclassOf<UAISense_Sight>());
		StimulusSourse->RegisterWithPerceptionSystem();
	}
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
		if (Target != nullptr && (EnemyState == EEnemyState::EES_Searching || EnemyState == EEnemyState::EES_Passive))
		{
			SetState(EEnemyState::EES_Searching);
			EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetPoint"), Target->GetActorLocation());
		}
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	if (HealthComponent->IsAlive())
	{
		HealthComponent->ReceiveDamage(DamageAmount);

		const float Stunned = FMath::FRandRange(0.f, 1.f);

		if (Stunned <= StunChance)
		{
			PlayHitMontage(FName("HitReactFront"));
			SetStunned(bStunned = true);
		}
		if (!HealthComponent->IsAlive())	Die();
	}
	return DamageAmount;
}

