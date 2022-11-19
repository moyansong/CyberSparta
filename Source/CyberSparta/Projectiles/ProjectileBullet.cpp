// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::ApplyDamage(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::ApplyDamage(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	ACharacter* MyCharacter = Cast<ACharacter>(GetOwner());
	if (MyCharacter)
	{
		AController* MyController = MyCharacter->Controller;
		if (MyController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, MyController, this, UDamageType::StaticClass());
		}
	}

}
