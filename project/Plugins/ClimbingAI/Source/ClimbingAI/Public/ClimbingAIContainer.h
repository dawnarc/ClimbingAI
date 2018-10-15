// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ClimbingAIContainer.generated.h"

class APawn;

/**
 * 
 */
UCLASS()
class CLIMBINGAI_API UClimbingAIContainer : public UObject
{
	GENERATED_BODY()

public:

	static void AddAgent(APawn* Pawn);

	static void RemoveAgent(APawn* Pawn);

	static const TArray<APawn*>& GetAgentList();

private:


	static TArray<APawn*> AgentList;
};
