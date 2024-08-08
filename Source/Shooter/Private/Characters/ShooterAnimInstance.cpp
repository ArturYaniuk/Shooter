// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterAnimInstance.h"
#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Public/Items/Weapons/Weapon.h"
#include "../Public/Items/Weapons/WeaponType.h"


UShooterAnimInstance::UShooterAnimInstance() :
	EquippedWeaponType(EWeaponType::ECS_MAX),
	GroundSpeed(0.f),
	IsFalling(false),
	bAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	bShoodUseFABRIK(false)
{
}

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
		bAiming = ShooterCharacter->GetAiming();

		if (ShooterCharacterMovement)
		{
			GroundSpeed = UKismetMathLibrary::VSizeXY(ShooterCharacterMovement->Velocity);
			IsFalling = ShooterCharacterMovement->IsFalling();

		}
		bShoodUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;

		bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
		CharacterState = ShooterCharacter->GetCharacterState();
		if (ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}
	TurnInPlace();

}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;
	
	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	
	if (GroundSpeed > 0 || bAiming || IsFalling || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress)
	{
		RootYawOffset = 0.f;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		CharacterYawLastFrame = CharacterYaw;
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta{ CharacterYaw - CharacterYawLastFrame };

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		const float Turning{ GetCurveValue(TEXT("Turning")) };

		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };

			if (ABSRootYawOffset > 30.f)
			{
				const float YawExcess{ ABSRootYawOffset - 30.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
	}
}
