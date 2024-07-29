// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterAnimInstance.h"
#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Public/Items/Weapons/Weapon.h"
#include "../Public/Items/Weapons/WeaponType.h"


void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	if (ShooterCharacter)
	{
		ShooterCharacterMovement = ShooterCharacter->GetCharacterMovement();
	}
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (ShooterCharacter)
	{
		if (ShooterCharacterMovement)
		{
			GroundSpeed = UKismetMathLibrary::VSizeXY(ShooterCharacterMovement->Velocity);
			IsFalling = ShooterCharacterMovement->IsFalling();

		}
		bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
		CharacterState = ShooterCharacter->GetCharacterState();
		if (ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}

}
