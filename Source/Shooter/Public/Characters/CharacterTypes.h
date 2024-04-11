#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquipedFirstWeapon UMETA(DisplayName = "Equiped First Weapon"),
	ECS_EquipedSecondWeapon UMETA(DisplayName = "Equiped Second Weapon")
};