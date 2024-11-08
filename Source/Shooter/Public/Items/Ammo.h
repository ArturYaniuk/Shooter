// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapons/AmmoType.h"
#include "Engine/DataTable.h"
#include "Ammo.generated.h"

USTRUCT()
struct FAmmoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* AmmoMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoQuantity;

};
/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

	void TakeParams(EAmmoType NewAmmoType);

protected:
	virtual void BeginPlay() override;

	virtual void SetItemProperties(EItemState State) override;

	UFUNCTION()
	virtual void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* AmmoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;


public:
	FORCEINLINE USkeletalMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE void SetAmmoType(EAmmoType NewAmmoType) { AmmoType = NewAmmoType; TakeParams(AmmoType); }
};
