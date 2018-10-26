// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ClimbWallGameMode.h"
#include "ClimbWallPlayerController.h"
#include "ClimbWallCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "UObjectGlobals.h"

#include "ClimbingAIComponent.h"

#include "ClimbWallCharacter.h"

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
		if (AClimbWallCharacter* Character = GetWorld()->SpawnActor<AClimbWallCharacter>(DefaultPawnClass, FVector(-490.f, 400.f, 292.f), FRotator::ZeroRotator))
		{
			Character->SpawnDefaultController();

			Character->SetCanMove(true);
			
			if (UClimbingAIComponent* Comp = NewObject<UClimbingAIComponent>(Character))
			{
				Comp->RegisterComponent();
				Comp->SetEnable(true);
				Comp->SetState(EClimbAIState::ECS_NotArrive);
				Comp->SetLangingForceLocal(FVector(250.f, 0.f, 1000.f));
				//Comp->SetLandingSectionName(FString("step_01"), FString("step_01"));
				Comp->SetLangingFinishZCoor(1180.f);

				UAnimMontage* ClimbMontage = LoadObject<UAnimMontage>(nullptr, TEXT("AnimMontage'/Game/Mannequin/Animations/ThirdPersonWalk_Montage.ThirdPersonWalk_Montage'"));
				if (ClimbMontage)
				{
					//启用RootMotion
					ClimbMontage->bEnableRootMotionTranslation = true;
					Comp->SetClimbAnimation(ClimbMontage);
				}

				//设置爬墙动画
				UAnimMontage* LandingMontage = LoadObject<UAnimMontage>(nullptr, TEXT("AnimMontage'/Game/Mannequin/Animations/ThirdPersonJump_Start_Montage.ThirdPersonJump_Start_Montage'"));
				if (LandingMontage)
				{
					//启用RootMotion
					LandingMontage->bEnableRootMotionTranslation = true;
					Comp->SetLandingAnimation(LandingMontage);
				}

				Comp->OnClimbAIStateChange().AddUObject(Character, &AClimbWallCharacter::OnClimbAIStateChange);
			}
		}
	}
}