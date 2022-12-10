#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Fist UMETA(DisplayName = "Fist"),
	EWT_Knife UMETA(DisplayName = "Knife"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),
	EWT_SpecialWeapon UMETA(DisplayName = "SpecialWeapon"),
	EWT_Max UMETA(DisplayName = "DefaultMax")
};