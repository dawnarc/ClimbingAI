// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingAIComponent.generated.h"

class UAnimMontage;
class AClimbingSplineActor;

UENUM(BlueprintType)
enum class EClimbAIState : uint8
{
	//未到达攀爬区域
	ECS_NotArrive,
	//已到达攀爬区域
	ECS_Arrived,
	//攀爬
	ECS_Climb,
	//跳墙着陆
	ECS_Landing,
	//跳墙着陆之后的相关处理（比如播放着陆动画）
	ECS_Landed,
	//达到墙上
	ECS_IdleOnWall,
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

	void SetEnable(bool bEnabled) { IsEnable = bEnabled; }

	void SetState(EClimbAIState ClimbState);

	EClimbAIState GetState() { return State; }

	void CalcClimbStartPoint(const FVector& Direction, float Distance, const FVector& LineStart, const FVector& LineEnd);

	void ResetRotateLerpTime() { ClimbRotateLerpTime = 0.f; }

	void EnablePawnCollision(bool bIsEnable);

	void SetClimbActor(const AClimbingSplineActor* ClimbingSplineActor) { ClimbActor = (AClimbingSplineActor*)ClimbingSplineActor; }

	void SetClimbAnimation(UAnimMontage* Anim) { ClimbMontage = Anim; }

	void SetClimbPause(bool bPause) { IsClimbPause = bPause; }

	void SetClimbSpeed(float Speed) { ClimbSpeed = Speed; }

	void SetLandingPause(bool bPause) { IsLandingPause = bPause; }

	void SetLandingAnimEnd(bool bIsEnd) { bIsLandingAnimEnd = bIsEnd; }

	void SetLandingAnimation(UAnimMontage* Anim) { LandingMontage = Anim; }

	void StopLandingAnimation();

	void SetLandingSectionName(const FString& Section01, const FString& Section02);

	void SetLangingForceLocal(const FVector& Force) { LangingForceLocal = Force; }

	void SetLangingFinishZCoor(float ZCoordinate) { LandingZCoordinate = ZCoordinate; }

	DECLARE_EVENT_OneParam(APawn, ClimbAIStateChangeEvent, EClimbAIState)

	ClimbAIStateChangeEvent& OnClimbAIStateChange() { return ClimbChangeEvent; }
	
protected:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

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

	//着陆
	void SMProcLanding(float DeltaSeconds);

	//抛物运动着陆（爬墙时）
	void ClimbLandingParabola(float DeltaSeconds);

protected:

	bool IsEnable = false;

	EClimbAIState State = EClimbAIState::ECS_NotArrive;

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

	//攀爬时转向的LerpTime
	float ClimbRotateLerpTime = 0.f;

	//爬墙的转向速率
	UPROPERTY(EditDefaultsOnly, Category = Climb)
		float ClimbRotateSpeedMul = 100.f;

	//爬墙Montage
	UAnimMontage* ClimbMontage = nullptr;
	//爬墙动画时长
	float ClimbAnimDuration = 0.f;

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
	FVector LangingForceLocal;

	//着陆起跳的初始速度（相对世界坐标的速度）
	FVector LangingForceWorld;

	//起跳时的坐标
	FVector LangingStartLocation;

	//重力加速度
	float GravityAcclerator = -980.f;

	//着陆过程的累积时长
	float AccumulateLangingTime = 0.f;

	//着陆的判断距离：着落点Z坐标相对起跳点Z坐标的差值
	float LandingFloorDistExtent = 100.f;

	//着陆动画
	UAnimMontage* LandingMontage = nullptr;
	//着陆动画的两个section name
	FString LandingMtgSecName01;
	FString LandingMtgSecName02;
	//着陆动画时长
	float LandingAnimDuration = 0.f;

	//墙顶着陆动画是否开始
	bool bIsLandingAnimStart = false;

	//墙顶着陆动画是否结束
	bool bIsLandingAnimEnd = false;

	//墙顶着陆动画已播放时长
	float ClimbLandingTime = 0.f;

	//上一帧的着陆累积长度
	float LastClimbLandingLen = 0.f;

	//着陆完成停留的Z坐标
	float LandingZCoordinate = 0.f;

	//ClimbAIState变化时的广播事件
	ClimbAIStateChangeEvent ClimbChangeEvent;
};
