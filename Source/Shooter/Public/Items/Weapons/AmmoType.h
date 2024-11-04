#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assault Rifle"),

	EAT_MAX UMETA(DisplayName = "DefaultMax")
};


UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	EPT_Default UMETA(DisplayName = "Default"),
	EPT_First UMETA(DisplayName = "First"),
	EPT_Second UMETA(DisplayName = "Second"),
	EPT_Something UMETA(DisplayName = "Someting")

};


UENUM(BlueprintType)
enum class EEnemyAmmoType : uint8
{
	EEAT_MainGun UMETA(DisplayName = "MainGun"),
	EEAT_Rocket UMETA(DisplayName = "Rocket"),

	EEAT_MAX UMETA(DisplayName = "DefaultMax")
};