// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UEnemyAnimInstance();
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:


	void TurnInPlace();

private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class AEnemy* Enemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;

	float EnemyYaw;

	float EnemyYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = YawOffset, meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	float RotationCurve;

	float RotationCurveLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Turn, meta = (AllowPrivateAccess = "true"))
	float Pitch;
};


