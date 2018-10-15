// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingAIComponent.generated.h"


UENUM(BlueprintType)
enum class EClimbState : uint8
{
	//未到达攀爬区域
	ECS_NotArrive,
	//已到达攀爬区域
	ECS_Arrived,
	//预备攀爬
	ECS_PreClimb,
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

	//抛物运动着陆（爬墙时）2018-09-22修改
	void ClimbLandingParabola(float DeltaSeconds);

	//未到达攀爬区域后
	void SMProcNotArrive(float DeltaSeconds);

	//到达攀爬区域
	void SMProcArrive(float DeltaSeconds);

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
};
