// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileSmokeGrenade.h"

void AProjectileSmokeGrenade::Destroyed()
{
	SimulateHit();
	AProjectile::Destroyed();
}
