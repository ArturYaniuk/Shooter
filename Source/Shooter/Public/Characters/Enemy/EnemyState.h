#pragma once

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Death UMETA(DisplayName = "Death"),
	EES_Stunned UMETA(DisplayName = "Stunned"),
	EES_Passive UMETA(DisplayName = "Passive"),
	EES_Searching UMETA(DisplayName = "Searcing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_MoveToTarget UMETA(DisplayName = "MoveToTarget"),

	EES_MAX UMETA(DisplayName = "DefaultMax")
};

