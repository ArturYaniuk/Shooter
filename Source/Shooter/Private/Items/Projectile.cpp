// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Projectile.h"
#include "./Characters/Enemy/Enemy.h"
#include "./Items/Weapons/Weapon.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
AProjectile::AProjectile()	:
	CollisionRadius(15.0f),
	CollisionHalfHeight(40.0f),
	InitialSpeed(3000.0f),
	MaxSpeed(3000.0f),
	bShouldBounce(true),
	Bounciness(0.5f),
	GravityScale(0.5f),
	LifeSpan(3.0f),
	ImpactImpulse(100.0f),
	HitDamage(1.0f),
	CritDamage(3.0f),
	WeaponDamageMultiplier(1.0f),
	WeaponCritDamageMultiplier(1.5f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));

	ProjectileMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ProjectileMeshComponent"));


	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SphereComponent"));
	// Set the sphere's collision profile name to "Projectile".
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	// Event called when component hits something.
	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	// Set the Capsule's collision radius.
	CollisionComponent->InitCapsuleSize(CollisionRadius, CollisionHalfHeight);
	// Set the root component to be the collision component.
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMeshComponent->SetupAttachment(RootComponent);

	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectile::TakeSpawnProperties(EProjectileType LocalProjectileType, float DamageMultiplier, float CritDamageMultiplier)
{
	const FString ProjectileTablePath{ TEXT("/Script/Engine.DataTable'/Game/DataTable/ProjectileDataTable.ProjectileDataTable'") };
	UDataTable* ProjectileTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ProjectileTablePath));

	if (ProjectileTableObject)
	{
		FProjectileDataTable* ProjectileDataRow = nullptr;
		switch (LocalProjectileType)
		{
		case EProjectileType::EPT_Default:
			ProjectileDataRow = ProjectileTableObject->FindRow<FProjectileDataTable>(FName("Default"), TEXT(""));
			break;
		case EProjectileType::EPT_First:
			ProjectileDataRow = ProjectileTableObject->FindRow<FProjectileDataTable>(FName("First"), TEXT(""));
			break;
		case EProjectileType::EPT_Second:
			ProjectileDataRow = ProjectileTableObject->FindRow<FProjectileDataTable>(FName("Second"), TEXT(""));
			break;
		case EProjectileType::EPT_Something:
			ProjectileDataRow = ProjectileTableObject->FindRow<FProjectileDataTable>(FName("Something"), TEXT(""));
			break;
		default:
			break;
		}
		if (ProjectileDataRow)
		{

			ProjectileType = ProjectileDataRow->ProjectileType;
			ProjectileMeshComponent->SetSkeletalMesh(ProjectileDataRow->ProjectileMeshComponent);
			ImpactParticles = ProjectileDataRow->ImpactParticles;
			CollisionRadius = ProjectileDataRow->CollisionRadius;
			CollisionHalfHeight = ProjectileDataRow->CollisionHalfHeight;
			InitialSpeed = ProjectileDataRow->InitialSpeed;
			MaxSpeed = ProjectileDataRow->MaxSpeed;
			bShouldBounce = ProjectileDataRow->bShouldBounce;
			Bounciness = ProjectileDataRow->Bounciness;
			GravityScale = ProjectileDataRow->GravityScale;
			LifeSpan = ProjectileDataRow->LifeSpan;
			ImpactImpulse = ProjectileDataRow->ImpactImpulse;
			HitDamage = ProjectileDataRow->HitDamage;
			CritDamage = ProjectileDataRow->CritDamage;
		}

		CollisionComponent->InitCapsuleSize(CollisionRadius, CollisionHalfHeight);
		ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = MaxSpeed;
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->bRotationRemainsVertical = true;
		ProjectileMovementComponent->bShouldBounce = bShouldBounce;
		ProjectileMovementComponent->Bounciness = Bounciness;
		ProjectileMovementComponent->ProjectileGravityScale = GravityScale;

		WeaponDamageMultiplier = DamageMultiplier;
		WeaponCritDamageMultiplier = CritDamageMultiplier;


		// Delete the projectile after 3 seconds.
		InitialLifeSpan = LifeSpan;
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::FireInDirection(const FVector& ShootDirection, EProjectileType Projectile, float DamageMultiplier, float CritDamageMultiplier)
{
	TakeSpawnProperties(Projectile, DamageMultiplier, CritDamageMultiplier);
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != nullptr && OtherActor != this && OtherComponent != nullptr && OtherComponent->IsSimulatingPhysics())
	{

		OtherComponent->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * ImpactImpulse, Hit.ImpactPoint);

	}

	if (OtherActor != nullptr && OtherActor != this && OtherComponent != nullptr)
	{

		FHitResult BeamHitResult;

		IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(OtherActor);
		if (BulletHitInterface)
		{
			BulletHitInterface->BulletHit_Implementation(Hit);
		}

		auto HitEnemy = Cast<APawn>(OtherActor);
		if (HitEnemy)
		{
			auto ShooterCharacterCritBone = Cast<AShooterCharacter>(OtherActor);
			auto EnemyCritBone = Cast<AEnemy>(OtherActor);
			if (Hit.GetActor() == ShooterCharacterCritBone)  CritBone = ShooterCharacterCritBone->GetCritBone();
			if (Hit.GetActor() == EnemyCritBone)  CritBone = EnemyCritBone->GetCritBone();

			if (Hit.BoneName.ToString() == CritBone)
			{
				Damage = CritDamage * WeaponCritDamageMultiplier;
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("HEADSHOT"));
			}
			else
			{
				Damage = HitDamage * WeaponDamageMultiplier;
			}

			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				HitEnemy->GetController(),
				Owner,
				UDamageType::StaticClass());

		}
	


		else
		{
			// Spawn default particles
			if (ImpactParticles)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					ImpactParticles,
					Hit.Location,
					Hit.ImpactPoint.Rotation());
			}
		}
		
    }
    Destroy(true);
}

