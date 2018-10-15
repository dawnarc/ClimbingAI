// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingAIUtil.h"
#include "ClimbingAIComponent.h"

UClimbingAIComponent* UClimbingAIUtil::GetClimbingAIComponent(AActor* Target)
{
	UClimbingAIComponent* Ret = nullptr;
	TArray<UActorComponent*> Components = Target->GetComponentsByClass(UClimbingAIComponent::StaticClass());

	if (Components.Num() > 0)
	{
		Ret = Cast<UClimbingAIComponent>(Components[0]);
	}
	return Ret;
}

