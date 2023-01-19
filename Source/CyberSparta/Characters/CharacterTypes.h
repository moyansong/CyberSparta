#pragma once

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	EHD_Forward UMETA(DisplayName = "Forward"),
	EHD_Backward UMETA(DisplayName = "Backward"),
	EHD_Left UMETA(DisplayName = "Left"),
	EHD_Right UMETA(DisplayName = "Right"),
	EWT_Max UMETA(DisplayName = "DefaultMax")
};