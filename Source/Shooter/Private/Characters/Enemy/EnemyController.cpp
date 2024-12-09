// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemy/EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISenseConfig_Sight.h"
#include "./Characters/Enemy/Enemy.h"
#include <Characters/ShooterCharacter.h>

AEnemyController::AEnemyController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	check(BlackboardComponent);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	check(BehaviorTreeComponent);


	SetupPerceptionSystem();
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn == nullptr) return;

	
	AEnemy* Enemy = Cast<AEnemy>(InPawn);
	if (Enemy)
	{
		if (Enemy->GetBehaviorTree())
		{
			
			BlackboardComponent->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
		}
	}
}

void AEnemyController::SetupPerceptionSystem()
{
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));


	if (SightConfig != nullptr) {
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
			TEXT("Perception Component")));
	}

	if (SightConfig != nullptr) {
		SightConfig->SightRadius = 1500.F;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.F;
		SightConfig->PeripheralVisionAngleDegrees = 90.F;
		SightConfig->SetMaxAge(
			5.F); // seconds - perceived stimulus forgotten after this time
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 1520.F;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::OnTargetDetected);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}

}

void AEnemyController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (auto *const Target = Cast<AShooterCharacter>(Actor))
	{
		GetBlackboardComponent()->SetValueAsBool(TEXT("bCanAttack"), Stimulus.WasSuccessfullySensed());
		GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Target);
		
	}
}
// TODO: move enemy control logic ti enemy controller