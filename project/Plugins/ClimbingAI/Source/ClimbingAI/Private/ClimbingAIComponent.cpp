// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingAIComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"
#include "Engine.h"

#include "ClimbingSplineActor.h"


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

	ClimbLandingParabola(DeltaTime);
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
	case EClimbState::ECS_Climb:
	{
		SMProcClimb(DeltaSeconds);
		break;
	}
	}
}

//朝墙转向（爬墙时）
void UClimbingAIComponent::RotateToWall(float DeltaSeconds)
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
		if (FVector::Dist(ClimbStartPoint, Pawn->GetActorLocation()) > 10)
		{
			if (ArriveLerpTime < ArriveLerpDuration)
			{
				ArriveLerpTime += DeltaSeconds;

				FVector NewLoc = FMath::Lerp<FVector>(LerpStartLocation, ClimbStartPoint, ArriveLerpTime / ArriveLerpDuration);

				Pawn->SetActorLocation(NewLoc);
			}
		}
		else
		{
			State = EClimbState::ECS_Climb;
			ClimbTime = 0.f;
			if (ClimbActor)
			{
				ClimbPointDiff = Pawn->GetActorLocation() - ClimbActor->GetSplineStartPointLocation();
			}
		}
	}
}

void UClimbingAIComponent::SMProcClimb(float DeltaSeconds)
{
	if (ClimbActor)
	{
		if (!IsClimbPause)
		{
			if (APawn* Char = Cast<APawn>(GetOuter()))
			{
				//开始攀爬
				ClimbTime += DeltaSeconds * ClimbSpeed;

				float SplineLen = ClimbActor->GetClimbSplineLen();

				float ClimbLen = (int)ClimbTime % (int)SplineLen;

				//如果已攀爬距离距离小于上一次记录的距离，则认为达到了攀爬终点
				if (ClimbLen >= LastClimbLen)
				{
					LastClimbLen = ClimbLen;

					if (USplineComponent* SplineComp = ClimbActor->GetSplineComponent())
					{
						FVector ClimbLoc = SplineComp->GetLocationAtDistanceAlongSpline(ClimbLen /*+ ClimbActor->GetStackAllLen()*/, ESplineCoordinateSpace::Type::World);
						FVector PawnLoc = ClimbLoc + ClimbPointDiff;
						Char->SetActorLocation(PawnLoc);
					}
				}
				else
				{
					//到达终点，越上城墙
					State = EClimbState::ECS_Landing;
					IsLandingPause = false;

					//设置起跳的初始速度
					FRotator Rot = Char->GetActorRotation();
					Rot.Roll = 0.f;
					Rot.Pitch = 0.f;

					LangingForceWorld = Rot.RotateVector(LangingForceLocal);

					LangingStartLocation = Char->GetActorLocation();
				}
			}
		}
	}
}

void UClimbingAIComponent::ClimbLandingParabola(float DeltaSeconds)
{
	if (EClimbState::ECS_Landing == State && !IsLandingPause)
	{
		AccumulateLangingTime += DeltaSeconds;

		float ZSpeed = GravityAcclerator * AccumulateLangingTime;

		//如果开始下降时，下降加速度加快，以增加视觉效果
		if (-ZSpeed > LangingForceWorld.Z)
		{
			ZSpeed = GravityAcclerator * 2 * AccumulateLangingTime;
		}

		FVector CurrSpeed = LangingForceWorld + FVector(0.f, 0.f, ZSpeed);

		FVector MoveDist = CurrSpeed * DeltaSeconds;

		if (AActor* Parent = Cast<AActor>(GetOuter()))
		{
			Parent->AddActorLocalOffset(MoveDist);
		}

		if (ACharacter* Char = Cast<ACharacter>(GetOuter()))
		{
			if (CurrSpeed.Z < 0 && Char->GetActorLocation().Z - LangingStartLocation.Z < LandingFloorDistExtent)
			{
				State = EClimbState::ESC_IdleOnWall;

				Char->SetActorEnableCollision(true);

				//Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
				//减小胶囊体半径，以减少卡住的情况（墙上的可行走空间较小）
				if (UCapsuleComponent* Capsule = Char->GetCapsuleComponent())
				{
					Capsule->SetCapsuleRadius(10.f);
				}

				if (UCharacterMovementComponent* Movement = Char->GetCharacterMovement())
				{
					Movement->SetMovementMode(EMovementMode::MOVE_Walking);
				}
			}
		}
	}
}