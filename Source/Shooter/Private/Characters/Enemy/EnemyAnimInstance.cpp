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
	if (Enemy != nullptr && Enemy->GetBShoudUseAnimOffset()) TurnInPlace();
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
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		Enemy->StopAnimMontage();
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

			if (GEngine) GEngine->AddOnScreenDebugMessage(2, -1, FColor::Cyan, FString::Printf(TEXT("DeltaRotation: %f"), DeltaRotation));
			if (GEngine) GEngine->AddOnScreenDebugMessage(4, -1, FColor::Blue, FString::Printf(TEXT("RotationCurve: %f"), RotationCurve));

			if (GEngine) GEngine->AddOnScreenDebugMessage(5, -1, FColor::Green, FString::Printf(TEXT("RotationCurveLastFrame: %f"), RotationCurveLastFrame));


			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };

			if (ABSRootYawOffset > 61.f)
			{
				const float YawExcess{ ABSRootYawOffset - 45.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		if (GEngine) GEngine->AddOnScreenDebugMessage(3, -1, FColor::Cyan, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
	}
}
