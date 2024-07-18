// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Items/Ammo.h"

AShooterCharacter::AShooterCharacter() :
	AutomaticFireRate(0.02f),
	bShouldFire(true),
	bFireButtonPressed(false),
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	sprintSpeed(1200.0f),
	defaultSpeed(600.0f),
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	CombatState(ECombatState::ECS_Unoccupied),
	bShouldPlayEquipSound(true),
	EquipSoundResetTime(0.2f)
{
	PrimaryActorTick.bCanEverTick = true;

	ShooterCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(ShooterCameraComponent != nullptr);

	ShooterCameraComponent->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));

	ShooterCameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f + BaseEyeHeight));

	ShooterCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetupAttachment(ShooterCameraComponent);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAmmoMap();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForItems();

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

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AShooterCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AShooterCharacter::StopSprint);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterCharacter::EKeyPressed);
	PlayerInputComponent->BindAction("Equip", IE_Released, this, &AShooterCharacter::EKeyReleased);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AShooterCharacter::DropWeapon);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

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
	GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
}

void AShooterCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = defaultSpeed;
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
	WeaponToEquip->Equip(GetMesh(), FName("RightHandSocket"));
	CharacterState = ECharacterState::ECS_EquipedFirstWeapon;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

	
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

	}
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocatin(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocatin, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

		OutBeamLocation = End;

		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		if (ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;

		}

		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation};
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}

	return false;
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
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	
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
		ReloadWeapon();
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
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

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
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquippedWeapon == nullptr) return;
	
	if (CarringAmmo()) 
	{
		CombatState = ECombatState::ECS_Reloading;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;
	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
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

void AShooterCharacter::ExchangeInventoryItem(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num()) || CombatState != ECombatState::ECS_Unoccupied) return;
	auto OldEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	if (OldEquippedWeapon->GetItemState() == EItemState::EIS_Pickup || NewWeapon->GetItemState() == EItemState::EIS_Pickup) return;

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(FName("Equip"));
	}
	NewWeapon->PlayEquipSound(this, true);
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
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
			ReloadWeapon();
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
