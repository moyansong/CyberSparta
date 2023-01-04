// Fill out your copyright notice in the Description page of Project Settings.


#include "SniperRifle.h"

ASniperRifle::ASniperRifle()
{
	Ammo = 10;
	MaxAmmo = 10;
	FireDelay = 1.f;
	bCanTarget = true;
	bCanAutomaticFire = false;
	bUseServerSideRewind = true;
	WeaponType = EWeaponType::EWT_SniperRifle;
}
