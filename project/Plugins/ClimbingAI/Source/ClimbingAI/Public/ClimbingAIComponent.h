// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingAIComponent.generated.h"

class AClimbingSplineActor;


UENUM(BlueprintType)
enum class EClimbState : uint8
{
	//未到达攀爬区域
	ECS_NotArrive,
	//已到达攀爬区域
	ECS_Arrived,
	//攀爬
	ECS_Climb,
	//跳墙着陆
	ECS_Landing,
	//达到墙上
	ESC_IdleOnWall,
};


/**
 * 
 */
UCLASS()
class CLIMBINGAI_API UClimbingAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UClimbingAIComponent();

	void SetState(EClimbState ClimbState) { State = ClimbState; }

	EClimbState GetState() { return State; }

	void CalcClimbStartPoint(const FVector& Direction, float Distance, const FVector& LineStart, const FVector& LineEnd);

	void EnablePawnCollision(bool bIsEnable);

	void SetEnable(bool bEnabled) { IsEnable = bEnabled; }

	void SetClimbActor(const AClimbingSplineActor* ClimbingSplineActor) { ClimbActor = (AClimbingSplineActor*)ClimbingSplineActor; }
	
protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	//@TODO inline
	//状态逻辑Tick（间隔0.3秒执行）
	void StateMachineTick(float DeltaSeconds);

	//状态执行逻辑Tick（每帧执行）
	void StateFrameTick(float DeltaSeconds);

	//朝墙转向（爬墙时）
	void RotateToWall(float DeltaSeconds);

	//未到达攀爬区域后
	void SMProcNotArrive(float DeltaSeconds);

	//到达攀爬区域
	void SMProcArrive(float DeltaSeconds);

	//攀爬
	void SMProcClimb(float DeltaSeconds);

	//抛物运动着陆（爬墙时）
	void ClimbLandingParabola(float DeltaSeconds);

protected:

	bool IsEnable = false;

	EClimbState State = EClimbState::ECS_NotArrive;

	//攀爬的起始点
	FVector ClimbStartPoint;

	//ECS_Arrived 状态下的Lerp时间
	float ArriveLerpTime = 0.f;
	float ArriveLerpDuration = 1.f;

	//ECS_Arrived 状态下的Lerp时间
	FVector LerpStartLocation;

	//FVector TestFootPerpendicular;

	//当前角色所属的ClimbActor
	AClimbingSplineActor* ClimbActor = nullptr;

	//爬墙是否暂停。用于优化爬墙动作
	bool IsClimbPause;

	//累积攀爬时长
	float ClimbTime;

	//爬墙速度倍率
	UPROPERTY(EditDefaultsOnly, Category = Climb)
		float ClimbSpeed = 240.f;

	//着陆速度倍率
	UPROPERTY(EditDefaultsOnly, Category = Climb)
		float CLIMB_LANDING_SPEED_MUL = 2.f;

	//上一帧的攀爬累积长度
	float LastClimbLen = 0.f;

	//实际攀爬路径点坐标和和SplineComponet曲线点坐标的间隔值
	FVector ClimbPointDiff;

	//着陆是否暂停。用于优化着陆动作
	bool IsLandingPause = true;

	//着陆起跳的初始速度（相对与自己为原点的速度）
	UPROPERTY(EditDefaultsOnly)
		FVector LangingForceLocal = FVector(200.f, 0.f, 1000.f);

	//着陆起跳的初始速度（相对世界坐标的速度）
	FVector LangingForceWorld;

	//起跳时的坐标
	FVector LangingStartLocation;

	//重力加速度
	float GravityAcclerator = -980.f;

	//着陆过程的累积时长
	float AccumulateLangingTime = 0.f;

	//着陆的判断距离：着落点Z坐标相对起跳点Z坐标的差值
	float LandingFloorDistExtent = 300.f;
};
