// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AttackComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "NiagaraFunctionLibrary.h"


// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAttackComponent::SpawnProjectile(const USkeletalMeshSocket* BarrelSocket, USkeletalMeshComponent* GunMesh, UNiagaraSystem* MuzzleFlash, FVector Direction, EProjectileType ProjectileType, float DamageMultiplier, float CritPointDamageMultiplier)
{
	const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GunMesh);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlash, SocketTransform.GetLocation(), GetOwner()->GetActorRotation());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetOwner()->GetInstigator();

	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), GetOwner()->GetActorRotation(), SpawnParams);

	if (Projectile) Projectile->FireInDirection(Direction, ProjectileType, DamageMultiplier, CritPointDamageMultiplier);
}


