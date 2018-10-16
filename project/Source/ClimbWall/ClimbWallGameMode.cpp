// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ClimbWallGameMode.h"
#include "ClimbWallPlayerController.h"
#include "ClimbWallCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "UObjectGlobals.h"

#include "ClimbingAIComponent.h"

AClimbWallGameMode::AClimbWallGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AClimbWallPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AClimbWallGameMode::StartPlay()
{
	Super::StartPlay();

	if (GetWorld())
	{
		if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, FVector(-490.f, 400.f, 292.f), FRotator::ZeroRotator))
		{
			Pawn->SpawnDefaultController();
			
			if (UClimbingAIComponent* Comp = NewObject<UClimbingAIComponent>(Pawn))
			{
				Comp->RegisterComponent();
				Comp->SetEnable(true);
				Comp->SetState(EClimbState::ECS_NotArrive);
			}
		}
	}
}