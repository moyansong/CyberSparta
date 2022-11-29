#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Idle UMETA(DisplayName = "Idle"),
	ECS_Firing UMETA(DisplayName = "Firing"),
	ECS_Reloading UMETA(DisplayName = "Fire"),
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};