// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeLauncher.h"

AGrenadeLauncher::AGrenadeLauncher()
{
	MaxAmmo = 10;
	Ammo = MaxAmmo;
	FireDelay = 0.5f;
	ReloadDuration = 2.f;
	bCanTarget = false;
	bUseLeftHandIK = true;
	bCanAutomaticFire = false;
	bUseServerSideRewind = false;
	bUseRightHandRotation = true;
	WeaponType = EWeaponType::EWT_GrenadeLauncher;
}
