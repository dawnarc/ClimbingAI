// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingAIContainer.h"

TArray<APawn*> UClimbingAIContainer::AgentList = TArray<APawn*>();

void UClimbingAIContainer::AddAgent(APawn* Pawn)
{
	AgentList.Add(Pawn);
}

void UClimbingAIContainer::RemoveAgent(APawn* Pawn)
{
	AgentList.Remove(Pawn);
}

const TArray<APawn*>& UClimbingAIContainer::GetAgentList()
{
	return AgentList;
}
