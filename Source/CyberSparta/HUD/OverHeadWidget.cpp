// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"
#include "Components/TextBlock.h"

void UOverHeadWidget::SetDisplayText(FString DisplayTextString)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(DisplayTextString));
	}
}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn, const FString& RoleType)
{
	ENetRole NetRole; 
	FString Role;
	if (InPawn)
	{
		if (RoleType == FString(TEXT("Local")))
		{
			NetRole = InPawn->GetLocalRole();
		}
		else
		{
			NetRole = InPawn->GetRemoteRole();
		}
	}
	switch (NetRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	FString NetRoleString = FString::Printf(TEXT("%s Role: %s"), *RoleType, *Role);
	SetDisplayText(NetRoleString);
}

void UOverHeadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
