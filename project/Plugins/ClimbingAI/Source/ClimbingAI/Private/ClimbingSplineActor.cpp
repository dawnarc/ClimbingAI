// Fill out your copyright notice in the Description page of Project Settings.

#include "ClimbingSplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"
#include "GameFramework/PawnMovementComponent.h"

#include "ClimbingAIUtil.h"
#include "ClimbingAIComponent.h"

// Sets default values
AClimbingSplineActor::AClimbingSplineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DelayCheckTime = FMath::RandRange(0.1f, 0.5f);

	CustomRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("CustomRootComp"));
	SetRootComponent(CustomRootComp);

	ClimbSplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("ClimbLineComp"));
	ClimbSplineComp->SetupAttachment(CustomRootComp);
	ClimbSplineComp->SetRelativeLocation(FVector(0.f, -ClimbDepth, 0.f));
	ClimbSplineComp->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));

	/*ClimbLineHorizonComp = CreateDefaultSubobject<USplineComponent>(TEXT("ClimbLineHorizonComp"));
	ClimbLineHorizonComp->SetupAttachment(ClimbLineVerticalComp);*/

	LeftEdgeComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftEdgeComp"));
	LeftEdgeComp->SetupAttachment(CustomRootComp);
	LeftEdgeComp->SetRelativeLocation(TempLeftEdgeLoc);
	LeftEdgeComp->SetRelativeRotation(TempLeftEdgeRot);
	LeftEdgeComp->SetArrowColor(ArrowColor);
	LeftEdgeComp->SetHiddenInGame(false);

	LeftEdgePassComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftEdgePassComp"));
	LeftEdgePassComp->SetupAttachment(CustomRootComp);
	LeftEdgePassComp->SetRelativeLocation(TempLeftEdgePassLoc);
	LeftEdgePassComp->SetRelativeRotation(TempLeftEdgePassRot);
	LeftEdgePassComp->SetArrowColor(ArrowColor);
	LeftEdgePassComp->SetHiddenInGame(false);

	RightEdgeComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightEdgeComp"));
	RightEdgeComp->SetupAttachment(CustomRootComp);
	RightEdgeComp->SetRelativeLocation(TempRightEdgeLoc);
	RightEdgeComp->SetRelativeRotation(TempRightEdgeRot);
	RightEdgeComp->SetArrowColor(ArrowColor);
	RightEdgeComp->SetHiddenInGame(false);

	RightEdgePassComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightEdgePassComp"));
	RightEdgePassComp->SetupAttachment(CustomRootComp);
	RightEdgePassComp->SetRelativeLocation(TempRightEdgePassLoc);
	RightEdgePassComp->SetRelativeRotation(TempRightEdgePassRot);
	RightEdgePassComp->SetArrowColor(ArrowColor);
	RightEdgePassComp->SetHiddenInGame(false);
}

void AClimbingSplineActor::Tick(float DeltaSeconds)
{
	if (!bEnabled)
	{
		return;
	}

	if (DelayCheckTime > 0.f)
	{
		DelayCheckTime -= DeltaSeconds;
	}
	else
	{
		FindAroundPawns(DeltaSeconds);

		EnterClimbAreaCheck(DeltaSeconds);
	}
}

void AClimbingSplineActor::BeginPlay()
{
	Super::BeginPlay();

	//计算攀爬总长度（过滤掉跳跃阶段和堆叠阶段）
	if (ClimbSplineComp)
	{
		int PointNum = ClimbSplineComp->GetNumberOfSplinePoints();

		ClimbLen = ClimbSplineComp->GetSplineLength();
	}

	if (RootComponent)
	{
		RootLocation = RootComponent->GetComponentLocation();
	}

	//计算箭头方向向量
	if (LeftEdgeComp)
	{
		LeftEdgeLocation = LeftEdgeComp->GetComponentLocation();
		LeftEdgeDirection = LeftEdgeComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (LeftEdgePassComp)
	{
		LeftEdgePassDirection = LeftEdgePassComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (RightEdgeComp)
	{
		RightEdgeLocation = RightEdgeComp->GetComponentLocation();
		RightEdgeDirection = RightEdgeComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (RightEdgePassComp)
	{
		RightEdgePassDirection = RightEdgePassComp->GetComponentRotation().Vector().GetSafeNormal();
	}

	//计算攀爬区域内的Lerp移动方向
	if (LeftEdgeComp && RightEdgeComp && ClimbSplineComp)
	{
		FVector Point1 = LeftEdgeComp->GetComponentLocation();
		FVector Point2 = RightEdgeComp->GetComponentLocation();
		SplineStartPointLoc = ClimbSplineComp->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

		FVector FootPerpendicular = UKismetMathLibrary::FindClosestPointOnLine(SplineStartPointLoc, Point1, Point1 - Point2);

		FVector Direction = SplineStartPointLoc - FootPerpendicular;
		ClimbEnterLerpDirection = Direction.GetSafeNormal();
		ClimbEnterLerpDistance = Direction.Size();
	}
}

void AClimbingSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (ClimbSplineComp)
	{
		ClimbSplineComp->SetRelativeLocation(FVector(0.f, -ClimbDepth, 0.f));
	}

	SetCustomProperties();
}

void AClimbingSplineActor::PostLoad()
{
	Super::PostLoad();

	//如果不在PostLoad中设置一次箭头的RelativeLocation，则ArrowComp在场景编辑器中的显示有问题，使用的还是构造函数中的相对坐标
	SetCustomProperties();
}

void AClimbingSplineActor::SetArrowRelativeLocatin(UArrowComponent* ArrowComp, const FVector& TemplateLoc, const FLinearColor& Color)
{
	if (ArrowComp)
	{
		ArrowComp->SetRelativeLocation(TemplateLoc);
		ArrowComp->SetArrowColor(Color);
	}
}

void AClimbingSplineActor::SetCustomProperties()
{
	SetArrowRelativeLocatin(LeftEdgeComp, FVector(-ClimbWidth / 2, TempLeftEdgeLoc.Y, TempLeftEdgeLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(LeftEdgePassComp, FVector(-ClimbWidth / 2, TempLeftEdgePassLoc.Y, TempLeftEdgePassLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(RightEdgeComp, FVector(ClimbWidth / 2, TempRightEdgeLoc.Y, TempRightEdgeLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(RightEdgePassComp, FVector(ClimbWidth / 2, TempRightEdgePassLoc.Y, TempRightEdgePassLoc.Z), ArrowColor);
}

void AClimbingSplineActor::FindAroundPawns(float DeltaSeconds)
{
	FindAroundPawnsTime += DeltaSeconds;

	if (FindAroundPawnsTime > FindAroundPawnsInterval)
	{
		FindAroundPawnsTime = 0.f;

		float CheckDist = ClimbWidth * 2;

		AroundPawns.Reset();

		for (TActorIterator<APawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (FVector::Dist(ActorItr->GetActorLocation(), RootLocation) < CheckDist)
			{
				AroundPawns.Add(*ActorItr);
			}
		}
	}
}

void AClimbingSplineActor::EnterClimbAreaCheck(float DeltaSeconds)
{
	EnterClimbAreaCheckTime += DeltaSeconds;

	if (EnterClimbAreaCheckTime > EnterClimbAreaCheckInterval)
	{
		EnterClimbAreaCheckTime = 0.f;

		for (APawn* Pawn : AroundPawns)
		{
			if (Pawn->IsValidLowLevelFast())
			{
				if (UClimbingAIComponent* Comp = UClimbingAIUtil::GetClimbingAIComponent(Pawn))
				{
					if (Comp->GetState() == EClimbState::ECS_NotArrive)
					{
						if (FVector::Dist(RootLocation, Pawn->GetActorLocation()) <= ClimbWidth)
						{
							//检测：箭头起点到角色坐标的向量，与两个箭头向量的夹角，如果与 LeftEdgePass 的夹角小于 LeftEdge 的夹角，则说明进入了攀爬区域。
							FVector LeftArrowToPawn = (Pawn->GetActorLocation() - LeftEdgeLocation).GetSafeNormal();

							float test1 = LeftEdgeDirection | LeftArrowToPawn;
							float test2 = LeftEdgePassDirection | LeftArrowToPawn;
							float LeftRadian = FMath::RadiansToDegrees(acosf(test1));
							float LeftRadianPass = FMath::RadiansToDegrees(acosf(test2));

							bool IsEnter = false;

							//如果左边检测没有进入攀爬区域，保险起见再检测一下右边箭头
							if (LeftRadian < LeftRadianPass)
							{
								FVector RightArrowToPawn = Pawn->GetActorLocation() - RightEdgeLocation;
								float RightRadian = acosf(RightEdgeDirection | RightArrowToPawn);
								float RightRadianPass = acosf(RightEdgePassDirection | RightArrowToPawn);
								if (RightRadian >= RightRadianPass)
								{
									IsEnter = true;
								}
							}
							else
							{
								IsEnter = true;
							}

							if (IsEnter)
							{
								if (LeftEdgeComp && RightEdgeComp)
								{
									Comp->CalcClimbStartPoint(ClimbEnterLerpDirection, ClimbEnterLerpDistance, LeftEdgeComp->GetComponentLocation(), RightEdgeComp->GetComponentLocation());
									Comp->SetState(EClimbState::ECS_Arrived);
									Comp->EnablePawnCollision(false);
									Comp->SetClimbActor(this);
									if (UPawnMovementComponent* MovementComp = Pawn->GetMovementComponent())
									{
										MovementComp->StopMovementImmediately();
									}
								}
							}
						}
					}
				}
			}
		}
	}
}