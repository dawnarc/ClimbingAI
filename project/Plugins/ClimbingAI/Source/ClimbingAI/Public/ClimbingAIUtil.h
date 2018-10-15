// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ClimbingAIUtil.generated.h"

class UClimbingAIComponent;

/**
 * 
 */
UCLASS()
class CLIMBINGAI_API UClimbingAIUtil : public UObject
{
	GENERATED_BODY()
	
public:
	
	static UClimbingAIComponent* GetClimbingAIComponent(AActor* Target);
	
};
