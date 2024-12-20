// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/EnemyAnimInstance.h"
#include "./Characters/Enemy/Enemy.h"
#include "Kismet/KismetMathLibrary.h"

UEnemyAnimInstance::UEnemyAnimInstance() :
	Speed(0.f),
	EnemyYaw(0.f),
	EnemyYawLastFrame(0.f),
	RootYawOffset(0.f)
{
}

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
	}
	
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (Enemy)
	{
		FVector Velocity{ Enemy->GetVelocity() };
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
	if (Enemy != nullptr) TurnInPlace();
	if (RootYawOffset > 61 || RootYawOffset < -61) Enemy->StopAnimMontage();

}

void UEnemyAnimInstance::TurnInPlace()
{
	if (Enemy == nullptr) return;

	Pitch = Enemy->GetBaseAimRotation().Pitch;

//	if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("Enemy->GetActorRotation().Yaw: %f"), Enemy->GetActorRotation().Yaw));
//	if (GEngine) GEngine->AddOnScreenDebugMessage(2, -1, FColor::Cyan, FString::Printf(TEXT("Enemy->GetBaseAimRotation().Pitch: %f"), Enemy->GetBaseAimRotation().Pitch));

	if (Speed > 0)
	{
		RootYawOffset = 0.f;
		EnemyYaw = Enemy->GetActorRotation().Yaw;
		EnemyYawLastFrame = EnemyYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		EnemyYawLastFrame = EnemyYaw;
		EnemyYaw = Enemy->GetActorRotation().Yaw;
		const float YawDelta{ EnemyYaw - EnemyYawLastFrame };
	//	if (GEngine) GEngine->AddOnScreenDebugMessage(8, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta: %f"), YawDelta));
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		const float Turning{ GetCurveValue(TEXT("Turning")) };

		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

	//		if (GEngine) GEngine->AddOnScreenDebugMessage(3, -1, FColor::Cyan, FString::Printf(TEXT("DeltaRotation: %f"), DeltaRotation));
	//		if (GEngine) GEngine->AddOnScreenDebugMessage(4, -1, FColor::Blue, FString::Printf(TEXT("RotationCurve: %f"), RotationCurve));

	//		if (GEngine) GEngine->AddOnScreenDebugMessage(5, -1, FColor::Green, FString::Printf(TEXT("RotationCurveLastFrame: %f"), RotationCurveLastFrame));


			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };

			if (ABSRootYawOffset > 61.f)
			{
				const float YawExcess{ ABSRootYawOffset - 45.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
//		if (GEngine) GEngine->AddOnScreenDebugMessage(6, -1, FColor::Cyan, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
	}
}
