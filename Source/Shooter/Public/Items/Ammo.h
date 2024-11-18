// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapons/AmmoType.h"
#include "Engine/DataTable.h"
#include "NiagaraComponent.h"
#include "Characters/Enemy/BulletHitInterface.h"
#include "Ammo.generated.h"

USTRUCT()
struct FAmmoDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* AmmoMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanBlowUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* BlowUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* BlowUpParticle;
};
/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

	void TakeParams(EAmmoType NewAmmoType);


	virtual void BulletHit_Implementation(FHitResult HitResult) override;



protected:
	virtual void BeginPlay() override;

	virtual void SetItemProperties(EItemState State) override;

	UFUNCTION()
	virtual void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	virtual void OnConstruction(const FTransform& Transform) override;

	void BlowUp();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* AmmoMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoBox", meta = (AllowPrivateAccess = "true"))
	bool CanBlowUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoBox", meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoBox", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AmmoBox", meta = (AllowPrivateAccess = "true"))
	class USoundCue* BlowUpSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoBox", meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* BlowUpParticle;

public:
	FORCEINLINE USkeletalMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE void SetAmmoType(EAmmoType NewAmmoType) { AmmoType = NewAmmoType; TakeParams(AmmoType); }
};
