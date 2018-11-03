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

	EdgeLeftFrontComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftEdgeComp"));
	EdgeLeftFrontComp->SetupAttachment(CustomRootComp);
	EdgeLeftFrontComp->SetRelativeLocation(TempEdgeLeftFrontLoc);
	EdgeLeftFrontComp->SetRelativeRotation(TempEdgeLeftFrontRot);
	EdgeLeftFrontComp->SetArrowColor(ArrowColor);

	EdgeLeftBackComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftEdgePassComp"));
	EdgeLeftBackComp->SetupAttachment(CustomRootComp);
	EdgeLeftBackComp->SetRelativeLocation(TempEdgeLeftBackLoc);
	EdgeLeftBackComp->SetRelativeRotation(TempEdgeLeftBackRot);
	EdgeLeftBackComp->SetArrowColor(ArrowColor);

	EdgeRightFrontComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightEdgeComp"));
	EdgeRightFrontComp->SetupAttachment(CustomRootComp);
	EdgeRightFrontComp->SetRelativeLocation(TempEdgeRightFrontLoc);
	EdgeRightFrontComp->SetRelativeRotation(TempEdgeRightFrontRot);
	EdgeRightFrontComp->SetArrowColor(ArrowColor);

	EdgeRightBackComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightEdgePassComp"));
	EdgeRightBackComp->SetupAttachment(CustomRootComp);
	EdgeRightBackComp->SetRelativeLocation(TempEdgeRightBackLoc);
	EdgeRightBackComp->SetRelativeRotation(TempEdgeRightBackRot);
	EdgeRightBackComp->SetArrowColor(ArrowColor);
}

void AClimbingSplineActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	if (EdgeLeftFrontComp)
	{
		EdgeLeftLocation = EdgeLeftFrontComp->GetComponentLocation();
		EdgeLeftFrontDirection = EdgeLeftFrontComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (EdgeLeftBackComp)
	{
		EdgeLeftBackDirection = EdgeLeftBackComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (EdgeRightFrontComp)
	{
		RightEdgeLocation = EdgeRightFrontComp->GetComponentLocation();
		EdgeRightFrontDirection = EdgeRightFrontComp->GetComponentRotation().Vector().GetSafeNormal();
	}
	if (EdgeRightBackComp)
	{
		EdgeRightBackDirection = EdgeRightBackComp->GetComponentRotation().Vector().GetSafeNormal();
	}

	//计算攀爬区域内的Lerp移动方向
	if (EdgeLeftFrontComp && EdgeRightFrontComp && ClimbSplineComp)
	{
		FVector Point1 = EdgeLeftFrontComp->GetComponentLocation();
		FVector Point2 = EdgeRightFrontComp->GetComponentLocation();
		SplineStartPointLoc = ClimbSplineComp->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

		FVector FootPerpendicular = UKismetMathLibrary::FindClosestPointOnLine(SplineStartPointLoc, Point1, Point1 - Point2);

		FVector Direction = SplineStartPointLoc - FootPerpendicular;
		ClimbEnterLerpDirection = Direction.GetSafeNormal();
		ClimbEnterLerpDistance = Direction.Size();
	}
}

#if WITH_EDITOR
void AClimbingSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (ClimbSplineComp)
	{
		ClimbSplineComp->SetRelativeLocation(FVector(0.f, -ClimbDepth, 0.f));
	}

	SetCustomProperties();
}
#endif

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
	SetArrowRelativeLocatin(EdgeLeftFrontComp, FVector(-ClimbWidth / 2, TempEdgeLeftFrontLoc.Y, TempEdgeLeftFrontLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(EdgeLeftBackComp, FVector(-ClimbWidth / 2, TempEdgeLeftBackLoc.Y, TempEdgeLeftBackLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(EdgeRightFrontComp, FVector(ClimbWidth / 2, TempEdgeRightFrontLoc.Y, TempEdgeRightFrontLoc.Z), ArrowColor);
	SetArrowRelativeLocatin(EdgeRightBackComp, FVector(ClimbWidth / 2, TempEdgeRightBackLoc.Y, TempEdgeRightBackLoc.Z), ArrowColor);
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
		PawnsWaitToClimb.Reset();

		for (APawn* Pawn : AroundPawns)
		{
			if (Pawn->IsValidLowLevelFast())
			{
				if (UClimbingAIComponent* Comp = UClimbingAIUtil::GetClimbingAIComponent(Pawn))
				{
					if (Comp->GetState() == EClimbAIState::ECS_NotArrive)
					{
						if (FVector::Dist(RootLocation, Pawn->GetActorLocation()) <= ClimbWidth / 2)
						{
							//检测：箭头起点到角色坐标的向量，与两个箭头向量的夹角，如果与 LeftEdgePass 的夹角小于 LeftEdge 的夹角，则说明进入了攀爬区域。
							FVector LeftArrowToPawn = (Pawn->GetActorLocation() - EdgeLeftLocation).GetSafeNormal();

							float LeftFrontAngle = EdgeLeftFrontDirection | LeftArrowToPawn;
							float LeftBackAngle = EdgeLeftBackDirection | LeftArrowToPawn;
							float LeftFrontRadian = FMath::RadiansToDegrees(acosf(LeftFrontAngle));
							float LeftBackRadian = FMath::RadiansToDegrees(acosf(LeftBackAngle));

							//如果左边检测没有进入攀爬区域，保险起见再检测一下右边箭头
							if (LeftFrontRadian < LeftBackRadian)
							{
								FVector RightArrowToPawn = Pawn->GetActorLocation() - RightEdgeLocation;
								float RightFrontRadian = acosf(EdgeRightFrontDirection | RightArrowToPawn);
								float RightBackRadian = acosf(EdgeRightBackDirection | RightArrowToPawn);
								if (RightFrontRadian >= RightBackRadian)
								{
									PawnsWaitToClimb.Add(Pawn);
								}
							}
							else
							{
								PawnsWaitToClimb.Add(Pawn);
							}
						}
					}
				}
			}
		}

		if (PawnsWaitToClimb.Num() > 0)
		{
			if (APawn* Pawn = PawnsWaitToClimb[FMath::RandRange(0, PawnsWaitToClimb.Num() - 1)])
			{
				if (UClimbingAIComponent* Comp = UClimbingAIUtil::GetClimbingAIComponent(Pawn))
				{
					if (EdgeLeftFrontComp && EdgeRightFrontComp)
					{
						Comp->CalcClimbStartPoint(ClimbEnterLerpDirection, ClimbEnterLerpDistance, EdgeLeftFrontComp->GetComponentLocation(), EdgeRightFrontComp->GetComponentLocation());
						Comp->SetState(EClimbAIState::ECS_Arrived);
						Comp->ResetRotateLerpTime();
						Comp->EnablePawnCollision(false);
						Comp->SetClimbActor(this);
						Comp->SetClimbPause(true);
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