// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Ammo.h"

#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/ShooterCharacter.h"

AAmmo::AAmmo():
	AmmoType(EAmmoType::EAT_MAX)
{
	RootComponent = GetItemMesh();
	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);

}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);

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


		break;
	case EItemState::EIS_Equipped:

		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_Falling:

		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetEnableGravity(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

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
		}
	
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (OtherActor)
	{
		auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->GetPickupItem(this);
		}
	}
}

void AAmmo::OnConstruction(const FTransform& Transform)
{
	TakeParams(AmmoType);
}
