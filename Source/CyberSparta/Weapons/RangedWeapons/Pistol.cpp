// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

APistol::APistol()
{
	Ammo = 7;
	MaxAmmo = 7;
	bCanTarget = false;
	bUseLeftHandIK = true;
	bCanAutomaticFire = false;
	bUseServerSideRewind = true;
	bUseRightHandRotation = true;
	WeaponType = EWeaponType::EWT_Pistol;
}
