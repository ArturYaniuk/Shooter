// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/EnemyAnimInstance.h"
#include "./Characters/Enemy/Enemy.h"
#include "Kismet/KismetMathLibrary.h"

UEnemyAnimInstance::UEnemyAnimInstance() :
	GroundSpeed(0.f),
	EnemyYaw(0.f),
	EnemyYawLastFrame(0.f),
	RootYawOffset(0.f)
{
}

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}
	if (Enemy)
	{
		FVector Velocity{ Enemy->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	TurnInPlace();
}

void UEnemyAnimInstance::TurnInPlace()
{
	if (Enemy == nullptr) return;

	Pitch = Enemy->GetBaseAimRotation().Pitch;

	if (GroundSpeed > 0)
	{
		RootYawOffset = 0.f;
		EnemyYaw = Enemy->GetActorRotation().Yaw;
		EnemyYawLastFrame = EnemyYaw;
	}
	else
	{
		EnemyYawLastFrame = EnemyYaw;
		EnemyYaw = Enemy->GetActorRotation().Yaw;
		const float YawDelta{ EnemyYaw - EnemyYawLastFrame };

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		const float Turning{ GetCurveValue(TEXT("Turning")) };

		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };

			if (ABSRootYawOffset > 45.f)
			{
				const float YawExcess{ ABSRootYawOffset - 45.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
	}
}
