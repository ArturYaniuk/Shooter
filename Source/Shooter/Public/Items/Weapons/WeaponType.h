#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_TempWeapon UMETA(DisplayName = "TempWeapon"),
	
	ECS_MAX UMETA(DisplayName = "DefaultMax")
};