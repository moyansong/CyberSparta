// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponWidget.h"
#include "Components/Image.h"
#include "../CyberSparta.h"
#include "../Weapons/Weapon.h"

void UWeaponWidget::SetHUDWeaponImage(AWeapon* LastWeapon, AWeapon* Weapon)
{
	if (!Weapon || !WeaponImage || !Weapon->WeaponImage) return;
	
	WeaponImage->SetBrushFromTexture(Weapon->WeaponImage);
}
