// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Ammo.h"

#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AAmmo::AAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

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
