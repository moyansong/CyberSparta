// Fill out your copyright notice in the Description page of Project Settings.


#include "Rifle.h"

ARifle::ARifle()
{
	Ammo = 32;
	MaxAmmo = 32;
	bCanTarget = true;
	bUseLeftHandIK = true;
	bCanAutomaticFire = true;
	bUseServerSideRewind = true;
	bUseRightHandRotation = true;
	WeaponType = EWeaponType::EWT_Rifle;
}
