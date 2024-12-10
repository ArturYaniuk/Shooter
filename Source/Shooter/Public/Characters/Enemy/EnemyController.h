// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIController.h"

#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyController();

	virtual void OnPossess(APawn* InPawn) override;

private:

	class AEnemy* Enemy;

	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	class UAISenseConfig_Sight* SightConfig;

	void SetupPerceptionSystem();

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus);

public:

	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }

};
