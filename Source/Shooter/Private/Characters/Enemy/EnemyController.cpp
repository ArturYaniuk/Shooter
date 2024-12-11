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

	Enemy = Cast<AEnemy>(InPawn);
	
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
		SightConfig->SightRadius = 1500.f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 100.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.f;
		SightConfig->SetMaxAge(
			1.F); // seconds - perceived stimulus forgotten after this time
		SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;
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
		if (Stimulus.WasSuccessfullySensed())
		{
			Enemy->SetState(EEnemyState::EES_MoveToTarget);
			MoveToActor(Target, 500.f);
			if (GetBlackboardComponent()->GetValueAsBool(TEXT("InAttackRange")))
			{
				StopMovement();
		
				Enemy->SetState(EEnemyState::EES_Attacking);
			}
			if (GEngine) GEngine->AddOnScreenDebugMessage(1, 1, FColor::Cyan, FString::Printf(TEXT("GetOwner()->GetDistanceTo(Actor): %f"), Enemy->GetDistanceTo(Target)));
			
		}
		else
		{
			StopMovement();
			GetBlackboardComponent()->SetValueAsVector(TEXT("TargetPoint"), Stimulus.StimulusLocation);
			Enemy->SetState(EEnemyState::EES_Searching);		
			GetBlackboardComponent()->SetValueAsBool(TEXT("bAttacking"), false);
			Enemy->StopAnimMontage();
		}
	}
}


