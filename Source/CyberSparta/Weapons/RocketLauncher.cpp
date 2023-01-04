// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketLauncher.h"

ARocketLauncher::ARocketLauncher()
{
	MaxAmmo = 1;
	Ammo = MaxAmmo;
	FireDelay = 1.f;
	ReloadDuration = 2.f;
	bCanTarget = true;
	bCanAutomaticFire = false;
	bUseServerSideRewind = false;
	WeaponType = EWeaponType::EWT_RocketLauncher;
}
