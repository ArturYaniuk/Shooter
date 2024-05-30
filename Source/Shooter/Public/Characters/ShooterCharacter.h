// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "CharacterTypes.h"
#include "items/Weapons/AmmoType.h"
#include "CombatState.h"
#include "ShooterCharacter.generated.h"

class AItem;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:

	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	UFUNCTION()
	void StartSprint();

	UFUNCTION()
	void StopSprint();

	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

	UPROPERTY(EditAnywhere, Category = MovementSpeed)
	float sprintSpeed;

	UPROPERTY(EditAnywhere, Category = MovementSpeed)
	float defaultSpeed;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ShooterCameraComponent;


	void EKeyPressed();
	void EKeyReleased();

	void DropWeapon();

	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	void FireButtonPressed();
	void FireButtonReleased();

	void StartFireTimer();

	UFUNCTION()
	void AutoFireReset();

	void InitializeAmmoMap();
	bool WeaponHasAmmo();

	void PlayFireSound();
	void SendBullet();
	void PlayGunFireMontage();

	void ReloadButtonPressed();
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	bool CarringAmmo();


private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combate", meta = (AllowPrivateAccess = "true"))

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	bool bFireButtonPressed;

	bool bShouldFire;

	float AutomaticFireRate;

	FTimerHandle AutoFireTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() { return CharacterState; }

};
