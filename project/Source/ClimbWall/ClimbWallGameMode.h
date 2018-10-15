// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ClimbWallGameMode.generated.h"

UCLASS(minimalapi)
class AClimbWallGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AClimbWallGameMode();

protected:

	virtual void StartPlay() override;
};



