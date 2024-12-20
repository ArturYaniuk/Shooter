// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Ammo.h"

#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/ShooterCharacter.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"

AAmmo::AAmmo():
	AmmoType(EAmmoType::EAT_MAX),
	CanBlowUp(false)
{
	RootComponent = GetItemMesh();
	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetSphereRadius(50.f);

}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	GetAreaSphere()->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);

}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);
	AmmoMesh = GetItemMesh();
	switch (State)
	{
	case EItemState::EIS_Pickup:

		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetAreaSphere()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		break;
	case EItemState::EIS_Equipped:

		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetAreaSphere()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		break;

	case EItemState::EIS_Falling:
		
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetEnableGravity(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		GetAreaSphere()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		break;
	}
}

void AAmmo::TakeParams(EAmmoType NewAmmoType)
{
	const FString AmmoTablePath{ TEXT("/Script/Engine.DataTable'/Game/DataTable/AmmoDataTable.AmmoDataTable'") };

	UDataTable* AmmoTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *AmmoTablePath));
	if (AmmoTableObject)
	{
		FAmmoDataTable* AmmoDataRow = nullptr;
		switch (NewAmmoType)
		{
		case EAmmoType::EAT_MAX:
			AmmoDataRow = AmmoTableObject->FindRow<FAmmoDataTable>(FName("Default"), TEXT(""));
			break;
		case EAmmoType::EAT_MainGun:
			AmmoDataRow = AmmoTableObject->FindRow<FAmmoDataTable>(FName("MainGun"), TEXT(""));
			break;
		case EAmmoType::EAT_Rocket:
			AmmoDataRow = AmmoTableObject->FindRow<FAmmoDataTable>(FName("Rocket"), TEXT(""));
			break;
		case EAmmoType::EAT_9mm:
			AmmoDataRow = AmmoTableObject->FindRow<FAmmoDataTable>(FName("9mm"), TEXT(""));
			break;
		case EAmmoType::EAT_AR:
			AmmoDataRow = AmmoTableObject->FindRow<FAmmoDataTable>(FName("AR"), TEXT(""));
			break;
		default:
			break;
		}
		if (AmmoDataRow)
		{
			GetItemMesh()->SetSkeletalMesh(AmmoDataRow->AmmoMeshComponent);
			SetItemCount(AmmoDataRow->AmmoQuantity);
			CanBlowUp = AmmoDataRow->CanBlowUp;
			Health = AmmoDataRow->Health;
			MaxHealth = AmmoDataRow->MaxHealth;
			BlowUpSound = AmmoDataRow->BlowUpSound;
			BlowUpParticle = AmmoDataRow->BlowUpParticle;
		}
	
	}
}


void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (OtherActor)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 37, FColor::Green, TEXT("FirstOverlap"));
		auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->GetPickupItem(this);
			return;
		}
		auto Projectile = Cast<AProjectile>(OtherActor);
		if (Projectile)
		{
			if ( Health - Projectile->GetDamage() <= 0)	BlowUp();
			else Health -= Projectile->GetDamage();
		}
	}
	
}

void AAmmo::OnConstruction(const FTransform& Transform)
{
	TakeParams(AmmoType);
}

void AAmmo::BlowUp()
{
	AController* MyOwnerInstgiator = this->GetInstigatorController();
	if (BlowUpSound) UGameplayStatics::PlaySoundAtLocation(this, BlowUpSound, GetActorLocation());
	if (BlowUpParticle) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BlowUpParticle, GetActorLocation());
	UGameplayStatics::ApplyRadialDamage(this, 1000, GetActorLocation(), 1000, UDamageType::StaticClass(), TArray<AActor*>(), this, MyOwnerInstgiator);
	
	Destroy();
}
