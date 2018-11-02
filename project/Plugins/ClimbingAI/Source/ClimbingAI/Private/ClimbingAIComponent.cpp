// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingAIComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"
#include "Engine.h"
#include "Animation/AnimMontage.h"

#include "ClimbingSplineActor.h"


UClimbingAIComponent::UClimbingAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClimbingAIComponent::SetState(EClimbAIState ClimbState)
{
	EClimbAIState OldState = State;
	State = ClimbState;

	if (OldState != ClimbState)
	{
		ClimbChangeEvent.Broadcast(ClimbState);
	}
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

void UClimbingAIComponent::StopLandingAnimation()
{
	if (LandingMontage)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
		{
			Character->StopAnimMontage(LandingMontage);
		}
	}
}

void UClimbingAIComponent::SetLandingSectionName(const FString& Section01, const FString& Section02)
{
	LandingMtgSecName01 = Section01;
	LandingMtgSecName02 = Section02;
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

	RotateToWall(DeltaTime);

	ClimbLandingParabola(DeltaTime);
}

//状态执行逻辑Tick（每帧执行）
void UClimbingAIComponent::StateFrameTick(float DeltaSeconds)
{
	switch (State)
	{
	case EClimbAIState::ECS_NotArrive:
	{
		//移动交给上层逻辑（这里的移动仅用作测试）
		//SMProcNotArrive(DeltaSeconds);
		break;
	}
	case EClimbAIState::ECS_Arrived:
	{
		SMProcArrive(DeltaSeconds);
		break;
	}
	case EClimbAIState::ECS_Climb:
	{
		SMProcClimb(DeltaSeconds);
		break;
	}
	case EClimbAIState::ECS_Landing:
	{
		SMProcLanding(DeltaSeconds);
		break;
	}
	case EClimbAIState::ECS_IdleOnWall:
	{
		SMProcLanding(DeltaSeconds);
		break;
	}
	}
}

//朝墙转向（爬墙时）
void UClimbingAIComponent::RotateToWall(float DeltaSeconds)
{
	//@TODO 有多个SplineActor时，角色需要需要正在攀爬的 SplineActor 绑定
	if (EClimbAIState::ECS_Arrived == State || EClimbAIState::ECS_Climb == State)
	{
		if (ClimbActor)
		{
			if (ACharacter* Parent = Cast<ACharacter>(GetOuter()))
			{
				FRotator CharRot = Parent->GetActorRotation();
				FRotator SplineRot = ClimbActor->GetActorRotation();
				SplineRot.Yaw -= 90.f;

				ClimbRotateLerpTime += DeltaSeconds * ClimbRotateSpeedMul;
				ClimbRotateLerpTime = ClimbRotateLerpTime > 1.f ? 1.f : ClimbRotateLerpTime;
				FRotator NewRot = FMath::Lerp(CharRot, SplineRot, ClimbRotateLerpTime);
				Parent->SetActorRotation(NewRot);
			}
		}
	}
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
			State = EClimbAIState::ECS_Climb;
			ClimbChangeEvent.Broadcast(EClimbAIState::ECS_Climb);

			ClimbTime = 0.f;
			if (ClimbActor)
			{
				ClimbPointDiff = Pawn->GetActorLocation() - ClimbActor->GetSplineStartPointLocation();
			}

			ClimbAnimDuration = 0.f;
		}
	}
}

void UClimbingAIComponent::SMProcClimb(float DeltaSeconds)
{
	if (ClimbActor)
	{
		if (!IsClimbPause)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
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
						Character->SetActorLocation(PawnLoc);
					}
				}
				else
				{
					//到达终点，越上城墙
					State = EClimbAIState::ECS_Landing;
					ClimbChangeEvent.Broadcast(EClimbAIState::ECS_Landing);

					LandingAnimDuration = 0.f;
					bIsLandingAnimStart = false;
					IsLandingPause = true;

					//设置起跳的初始速度
					FRotator Rot = Character->GetActorRotation();
					Rot.Roll = 0.f;
					Rot.Pitch = 0.f;
					//起跳的方向左右稍微随机一下角度
					Rot.Yaw += FMath::RandRange(-20.f, 20.f);

					LangingForceWorld = Rot.RotateVector(LangingForceLocal);

					LangingStartLocation = Character->GetActorLocation();
				}
			}
		}
	}

	if (ClimbMontage)
	{
		if (0.f == ClimbAnimDuration)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
			{
				ClimbAnimDuration = Character->PlayAnimMontage(ClimbMontage);
			}
		}
	}
}

void UClimbingAIComponent::SMProcLanding(float DeltaSeconds)
{
	if (!bIsLandingAnimStart)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
		{
			if (USkeletalMeshComponent *Mesh = Character->GetMesh())
			{
				if (UAnimInstance *AnimInstance = Mesh->GetAnimInstance())
				{
					if (LandingMontage)
					{
						bIsLandingAnimStart = true;
						bIsLandingAnimEnd = false;

						ClimbLandingTime = 0.f;
						LastClimbLandingLen = 0.f;

						Mesh->GlobalAnimRateScale = 0.7f;

						//播放着陆动画的第一个section
						Character->PlayAnimMontage(LandingMontage);
						if (LandingMtgSecName01.Len() > 0)
						{
							AnimInstance->Montage_JumpToSection(FName(*LandingMtgSecName01), LandingMontage);
						}
					}
				}
			}
		}
	}

	if (LandingMontage)
	{
		if (0.f == LandingAnimDuration)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
			{
				LandingAnimDuration = Character->PlayAnimMontage(LandingMontage);
			}
		}
	}
}

void UClimbingAIComponent::ClimbLandingParabola(float DeltaSeconds)
{
	if (EClimbAIState::ECS_Landing == State && !IsLandingPause)
	{
		AccumulateLangingTime += DeltaSeconds;

		float ZSpeed = GravityAcclerator * AccumulateLangingTime;

		//如果开始下降时，下降加速度翻倍，以增加视觉效果
		if (-ZSpeed > LangingForceWorld.Z)
		{
			ZSpeed = GravityAcclerator * 1.5 * AccumulateLangingTime;
		}

		FVector CurrSpeed = LangingForceWorld + FVector(0.f, 0.f, ZSpeed);

		FVector MoveDist = CurrSpeed * DeltaSeconds;

		if (AActor* Parent = Cast<AActor>(GetOuter()))
		{
			Parent->AddActorLocalOffset(MoveDist);
		}

		if (ACharacter* Character = Cast<ACharacter>(GetOuter()))
		{
			if (CurrSpeed.Z < 0 && FMath::Abs(Character->GetActorLocation().Z - LandingZCoordinate) < LandingFloorDistExtent)
			{
				State = EClimbAIState::ECS_Landed;

				//Char->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
				//减小胶囊体半径，以减少卡住的情况（墙上的可行走空间较小）
				if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
				{
					Capsule->SetCapsuleRadius(10.f);
				}

				//落地后movement切换为Walking
				if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
				{
					Movement->SetMovementMode(EMovementMode::MOVE_Walking);
				}

				//落地后启用碰撞
				Character->SetActorEnableCollision(true);

				//为了防止穿地，强制将Z坐标设置为墙顶的坐标
				Character->SetActorLocation(FVector(Character->GetActorLocation().X, Character->GetActorLocation().Y, LandingZCoordinate), true);

				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					//播放着陆动画的第二个section
					if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
					{
						if (LandingMontage)
						{
							Character->PlayAnimMontage(LandingMontage);
							if (LandingMtgSecName02.Len() > 0)
							{
								AnimInstance->Montage_JumpToSection(FName(*LandingMtgSecName02), LandingMontage);
							}
						}
					}
				}
			}
		}
	}
}