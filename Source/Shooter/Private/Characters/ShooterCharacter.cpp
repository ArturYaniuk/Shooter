// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/Enemy/Enemy.h"
#include "NiagaraFunctionLibrary.h"
#include "Items/Ammo.h"


AShooterCharacter::AShooterCharacter() :
	//Fire
	bShouldFire(true),
	bFireButtonPressed(false),
	//Crosshair setup
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	//Movement
	SprintSpeed(1200.0f),
	DefaultSpeed(600.0f),
	CurrentSpeed(600.0f),
	bSprinting(false),
	bCrouching(false),
	//Aiming setup
	ZoomInterpSpeed(0.f),
	bAiming(false),	
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(60.f),
	CameraCurrentFOV(0.f),
	//Camera setup
	DefaultCameraPosition(0.0f, 0.0f, 35.0f),
	CrouchCameraPosition(0.0f, 0.0f, 0.0f),
	CurrentCameraPosition(0.0f, 0.0f, 0.0f),
	//Default weapon prop
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	//Trace item
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	//States
	CombatState(ECombatState::ECS_Unoccupied),
	CharacterState(ECharacterState::ECS_Unequipped),
	//Sounds
	bShouldPlayEquipSound(true),
	EquipSoundResetTime(0.2f)
{
	PrimaryActorTick.bCanEverTick = true;

	ShooterCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(ShooterCameraComponent != nullptr);

	ShooterCameraComponent->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));

	ShooterCameraComponent->bUsePawnControlRotation = true;
//	ShooterCameraComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("b_rootSocket")));
	ShooterCameraComponent->SetupAttachment(RootComponent);


	//Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAmmoMap();
	if (ShooterCameraComponent)
	{
		CameraDefaultFOV = GetShooterCameraComponent()->FieldOfView;
		CameraCurrentFOV = CameraCurrentFOV;
	}
	ShooterCameraComponent->SetRelativeLocation(DefaultCameraPosition);
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForItems();
	CameraInterpZoom(DeltaTime);
	ChangeSpeed();
	CalculateCrosshairSpread(DeltaTime);

}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJump);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimigButtonReleased);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AShooterCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AShooterCharacter::StopSprint);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterCharacter::EKeyPressed);
	PlayerInputComponent->BindAction("Equip", IE_Released, this, &AShooterCharacter::EKeyReleased);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AShooterCharacter::DropWeapon);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this,&AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
	PlayerInputComponent->BindAction("6Key", IE_Pressed, this, &AShooterCharacter::SixKeyPressed);



}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && (Value != 0.f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && (Value != 0.f))
	{
		FVector Right = GetActorRightVector();
		AddMovementInput(Right, Value);
	}
}

void AShooterCharacter::StartJump()
{
	bPressedJump = true;
}

void AShooterCharacter::StopJump()
{
	bPressedJump = false;
}

void AShooterCharacter::StartSprint()
{
	bSprinting = true;
}

void AShooterCharacter::StopSprint()
{
	bSprinting = false;
}

void AShooterCharacter::EKeyPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (TraceHitItem)
	{
		GetPickupItem(OverlappingItem);
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::EKeyReleased()
{

}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (EquippedWeapon == nullptr)
	{
		// -1 == no EquippedWeapon yet. No need to reverse the icon animation
		EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
	}
	else
	{
		EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
	}
	CharacterState = WeaponToEquip->GetWDCharacterState();
	WeaponToEquip->Equip(GetMesh(), FName("RightHandSocket"));
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	StopAiming();
	
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		if (EquippedWeapon->GetItemState() == EItemState::EIS_Falling || EquippedWeapon->GetItemState() == EItemState::EIS_Pickup) return;
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		CharacterState = ECharacterState::ECS_Unequipped;
		EquippedWeapon->ThrowWeapon();
		Inventory.Remove(EquippedWeapon);
		StopAiming();

	}
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SpawnProjectile();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
		StartCrosshairBulletFire();
	}
}

void AShooterCharacter::AimingButtonPressed()
{
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Equipping) return;
	bAiming = true;
}

void AShooterCharacter::AimigButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::StopAiming()
{
	if (bAiming)
	{
		bAiming = false;
	}
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Set Current camera field of view
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetShooterCameraComponent()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	//Calculate crosshair Velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}
	//Calculate crosshair aim factor
	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}
	//Calculate crosshair firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTime, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::ChangeSpeed()
{
	if (bSprinting && !bAiming)
	{
		CurrentSpeed = SprintSpeed;
	}
	else if (bAiming)
	{
		CurrentSpeed = DefaultSpeed;
	}
	else
	{
		CurrentSpeed = DefaultSpeed;
	}
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
	
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else 
	{
		
	}


	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - WeaponTraceStart };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(
		OutHitResult,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);

	if (!OutHitResult.bBlockingHit) 
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
	
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
	
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon(EquippedWeapon);
	}
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr)	return false;
	
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

/*void AShooterCharacter::SendBullet()
{
	// Send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFash())
		{
			//(GetWorld(), EquippedWeapon->GetMuzzleFash(), SocketTransform);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFash(), SocketTransform.GetLocation(), this->GetViewRotation());
		}

		FHitResult BeamHitResult;

		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		if (bBeamEnd)
		{
			// Does hit Actor implement BulletHitInterface?
			if (BeamHitResult.GetActor())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{

					float HitDamage = EquippedWeapon->GetDamage();
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetCritBone())
					{
						HitDamage = EquippedWeapon->GetCritPointDamage();
					}
					else
					{
						HitDamage = EquippedWeapon->GetDamage();
					}

						UGameplayStatics::ApplyDamage(
							BeamHitResult.GetActor(),
							HitDamage,
							GetController(),
							this,
							UDamageType::StaticClass());

				}
			}
					
			
			else
			{
				// Spawn default particles
				if (ImpactParticles)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(),
						ImpactParticles,
						BeamHitResult.Location);
				}
			}


			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}
	}
}
*/

void AShooterCharacter::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon(EquippedWeapon);
}

void AShooterCharacter::ReloadWeapon(AWeapon* Weapon,bool pocketReload)
{
	if (Weapon == nullptr) return;
	if (pocketReload)
	{
		GetWorldTimerManager().SetTimer(isPocketReloading, this, &AShooterCharacter::StartPocketReload, 1.0f);
	}
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (!pocketReload)
	{
		if (CarringAmmo() && !EquippedWeapon->ClipIsFull()) 
			{
				CombatState = ECombatState::ECS_Reloading;

				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				StopAiming();
				if (AnimInstance && ReloadMontage)
				{
					AnimInstance->Montage_Play(ReloadMontage);
					AnimInstance->Montage_JumpToSection(Weapon->GetReloadMontageSection());
				}
			}
	}
	
	
}

void AShooterCharacter::FinishReloading(AWeapon* Weapon)
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (Weapon == nullptr) return;
	const auto AmmoType{ Weapon->GetAmmoType() };

	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagEmptySpace = Weapon->GetMagazineCapacity() - Weapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// reload the magazine with all the ammo we are carrying
			Weapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			Weapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

bool AShooterCharacter::CarringAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrossHairWorldDirection;

	//Get world position and direction of crosshairs

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), 
		CrosshairLocation, 
		CrosshairWorldPosition, 
		CrossHairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrossHairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{

			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_Equipped)
			{
				TraceHitItem = nullptr;
			}

			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

void AShooterCharacter::OneKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::FourKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::SixKeyPressed()
{
	if (!EquippedWeapon || EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItem(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
		ShooterCameraComponent->SetRelativeLocation(CurrentCameraPosition = bCrouching ? CurrentCameraPosition = CrouchCameraPosition : CurrentCameraPosition = DefaultCameraPosition);
	}

}

void AShooterCharacter::ExchangeInventoryItem(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || CombatState != ECombatState::ECS_Unoccupied) return;
	OldEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	if (OldEquippedWeapon->GetItemState() == EItemState::EIS_Pickup || NewWeapon->GetItemState() == EItemState::EIS_Pickup) return;

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && EquipMontage)
	{
		StopAiming();
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(FName("Equip"));
	}
	NewWeapon->PlayEquipSound(this, true);
	ReloadWeapon(OldEquippedWeapon, true);
	CharacterState = NewWeapon->GetWDCharacterState();
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("b_LeftHand")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);



}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::SpawnProjectile()
{
	// Send bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFash())
		{
			//(GetWorld(), EquippedWeapon->GetMuzzleFash(), SocketTransform);
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFash(), SocketTransform.GetLocation(), this->GetViewRotation());
		}

		FHitResult BeamHitResult;

		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), this->GetViewRotation(), SpawnParams);

		if (Projectile) Projectile->FireInDirection(this->GetViewRotation().Vector(), EquippedWeapon->GetProjectileType(), EquippedWeapon->GetDamageMultiplier(), EquippedWeapon->GetCritPointDamageMultiplier());
				
	}
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterCharacter::StartPocketReload()
{
	if (OldEquippedWeapon)
	{
		FinishReloading(OldEquippedWeapon);
	}
	
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);

}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}
	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		//Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon(EquippedWeapon);
		}
		
	}
	Ammo->Destroy();
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(Item);

	if (OverlappingWeapon)
	{	
		if (!EquippedWeapon && Inventory.Num() == 0)
		{
			EquipWeapon(OverlappingWeapon);
			Inventory.Add(OverlappingWeapon);
			EquippedWeapon->SetSlotIndex(0);
		}
		else if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			OverlappingWeapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(OverlappingWeapon);
			OverlappingWeapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else {

			SwapWeapon(OverlappingWeapon);
			
		}
		Item->PlayEquipSound(this);
	}


	auto Ammo = Cast<AAmmo>(Item);
	if (EquippedWeapon)
	{
		if (Ammo)
			{
				PickupAmmo(Ammo);
				Item->PlayEquipSound(this);
			}
	}
}
