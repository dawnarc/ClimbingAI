// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClimbingSplineActor.generated.h"

class USplineComponent;
class UArrowComponent;
class UMyArrowComponent;


UCLASS()
class CLIMBINGAI_API AClimbingSplineActor : public AActor
{
	GENERATED_BODY()

public:

	AClimbingSplineActor();

	float GetClimbSplineLen() { return ClimbLen; }

	USplineComponent* GetSplineComponent() { return ClimbSplineComp; }

	FVector GetSplineStartPointLocation() { return SplineStartPointLoc; }

protected:

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;

private:

	//设置箭头的相对坐标
	FORCEINLINE void SetArrowRelativeLocatin(UArrowComponent* ArrowComp, const FVector& TemplateLoc, const FLinearColor& Color);

	//设置自定义属性
	FORCEINLINE void SetCustomProperties();

	//查找当前ClimbingSplineActor附近的Pawn（预计算） @TODO inline
	void FindAroundPawns(float DeltaSeconds);

	//检测是否进入了攀爬区域 @TODO inline
	void EnterClimbAreaCheck(float DeltaSeconds);

protected:

	UPROPERTY(EditAnywhere)
		bool bEnabled = true;

	//检测延迟时间（用于保证各个ClimbingSplineActor检测逻辑的时间错开）
	float DelayCheckTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USceneComponent* CustomRootComp;

	//爬墙区域的路径
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USplineComponent* ClimbSplineComp;

	//RootComponent的世界坐标
	FVector RootLocation;

	//攀爬曲线的起始坐标
	FVector SplineStartPointLoc;

	//判断是否进入攀爬区域的两个箭头组件（左边）
	UArrowComponent* EdgeLeftFrontComp;
	UArrowComponent* EdgeLeftBackComp;

	//左边箭头的世界坐标
	FVector EdgeLeftLocation;

	//判断是否进入攀爬区域的两个箭头的方向向量（左边）
	FVector EdgeLeftFrontDirection;
	FVector EdgeLeftBackDirection;

	//判断是否进入攀爬区域的两个箭头组件（右边）
	UArrowComponent* EdgeRightFrontComp;
	UArrowComponent* EdgeRightBackComp;

	//右边箭头的世界坐标
	FVector EdgeRightLocation;

	//判断是否进入攀爬区域的两个方向向量（右边）
	FVector EdgeRightFrontDirection;
	FVector EdgeRightBackDirection;

	//攀爬区域检测的宽度
	UPROPERTY(EditAnywhere, Category = ArrowInfo)
		float ClimbWidth = 200.f;

	//攀爬区域检测的深度（水平方向，从RootComponent到 ClimbLineComp 之间的距离）
	UPROPERTY(EditAnywhere, Category = ArrowInfo)
		float ClimbDepth = 50.f;

	//箭头颜色
	UPROPERTY(EditAnywhere, Category = ArrowInfo)
		FLinearColor ArrowColor = FLinearColor::Yellow;

	//左边箭头的模版transform
	const FVector TempEdgeLeftFrontLoc = FVector(-ClimbWidth / 2, 0.f, 0.f);
	const FRotator TempEdgeLeftFrontRot = FRotator(0.f, 1.f, 0.f);
	const FVector TempEdgeLeftBackLoc = FVector(-ClimbWidth / 2, 0.f, 0.f);
	const FRotator TempEdgeLeftBackRot = FRotator(0.f, -1.f, 0.f);

	//右边箭头的模版transform
	const FVector TempEdgeRightFrontLoc = FVector(ClimbWidth / 2, 0.f, 0.f);
	const FRotator TempEdgeRightFrontRot = FRotator(0.f, 179.f, 0.f);
	const FVector TempEdgeRightBackLoc = FVector(ClimbWidth / 2, 0.f, 0.f);
	const FRotator TempEdgeRightBackRot = FRotator(0.f, -179.f, 0.f);

	//攀爬区域内的Lerp移动方向
	FVector ClimbEnterLerpDirection;
	
	//攀爬区域内的Lerp移动方向
	float ClimbEnterLerpDistance;

	//当前ClimbingSplineActor附近的Pawn（预计算）
	TArray<APawn*> AroundPawns;

	float FindAroundPawnsInterval = 3.f;
	float FindAroundPawnsTime = 0.f;

	float EnterClimbAreaCheckInterval = 5.f;
	float EnterClimbAreaCheckTime = 0.f;

	//攀爬总长度
	float ClimbLen;

	//已经符合攀爬条件等待攀爬的Pawn（用于随机筛选）
	TArray<APawn*> PawnsWaitToClimb;
};
