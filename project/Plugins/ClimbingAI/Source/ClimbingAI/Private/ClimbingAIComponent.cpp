// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingAIComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"


UClimbingAIComponent::UClimbingAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClimbingAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsEnable)
	{
		return;
	}

	StateFrameTick(DeltaTime);

	//DrawDebugSphere(GetWorld(), TestFootPerpendicular, 10, 10, FColor::Green);
}

void UClimbingAIComponent::CalcClimbStartPoint(const FVector& Direction, float Distance, const FVector& LineStart, const FVector& LineEnd)
{
	if (AActor* Pawn = Cast<AActor>(GetOuter()))
	{
		FVector FootPerpendicular = UKismetMathLibrary::FindClosestPointOnLine(Pawn->GetActorLocation(), LineStart, LineStart - LineEnd);

		ClimbStartPoint = FootPerpendicular + Direction * Distance;
		LerpStartLocation = Pawn->GetActorLocation();

		//TestFootPerpendicular = ClimbStartPoint;
	}
}

void UClimbingAIComponent::EnablePawnCollision(bool bIsEnable)
{
	if (AActor* Pawn = Cast<AActor>(GetOuter()))
	{
		Pawn->SetActorEnableCollision(bIsEnable);
	}
}

void UClimbingAIComponent::StateMachineTick(float DeltaSeconds)
{

}

//状态执行逻辑Tick（每帧执行）
void UClimbingAIComponent::StateFrameTick(float DeltaSeconds)
{
	switch (State)
	{
	case EClimbState::ECS_NotArrive:
	{
		SMProcNotArrive(DeltaSeconds);
		break;
	}
	case EClimbState::ECS_Arrived:
	{
		SMProcArrive(DeltaSeconds);
		break;
	}
	}
}

//朝墙转向（爬墙时）
void UClimbingAIComponent::RotateToWall(float DeltaSeconds)
{

}

//抛物运动着陆（爬墙时）2018-09-22修改
void UClimbingAIComponent::ClimbLandingParabola(float DeltaSeconds)
{

}

void UClimbingAIComponent::SMProcNotArrive(float DeltaSeconds)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
	{
		FVector Direction = Character->GetActorRotation().Vector().GetSafeNormal();
		Character->AddMovementInput(Direction);
	}
}

void UClimbingAIComponent::SMProcArrive(float DeltaSeconds)
{
	if (AActor* Pawn = Cast<AActor>(GetOuter()))
	{
		if (ArriveLerpTime < ArriveLerpDuration)
		{
			ArriveLerpTime += DeltaSeconds;

			FVector NewLoc = FMath::Lerp<FVector>(LerpStartLocation, ClimbStartPoint, ArriveLerpTime / ArriveLerpDuration);

			Pawn->SetActorLocation(NewLoc);
		}
	}
}