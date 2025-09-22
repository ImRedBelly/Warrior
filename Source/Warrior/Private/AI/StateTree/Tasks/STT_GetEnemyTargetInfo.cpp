// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/StateTree/Tasks/STT_GetEnemyTargetInfo.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"

USTT_GetEnemyTargetInfo::USTT_GetEnemyTargetInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true;
}

EStateTreeRunStatus USTT_GetEnemyTargetInfo::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (auto EnemyPerceptionComponent = EnemyCharacter->GetController<AAIController>()->GetAIPerceptionComponent())
	{
		EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &USTT_GetEnemyTargetInfo::OnEnemyPerceptionUpdated);
	}

	CalculateValues();

	return EStateTreeRunStatus::Running;
}

void USTT_GetEnemyTargetInfo::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (auto EnemyPerceptionComponent = EnemyCharacter->GetController<AAIController>()->GetAIPerceptionComponent())
	{
		EnemyPerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}

	Super::ExitState(Context, Transition);
}

EStateTreeRunStatus USTT_GetEnemyTargetInfo::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	CalculateValues();
	return EStateTreeRunStatus::Running;
}

void USTT_GetEnemyTargetInfo::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed() && Actor)
	{
		PlayerCharacter = Cast<ACharacter>(Actor);
	}
}

void USTT_GetEnemyTargetInfo::CalculateValues()
{
	if (IsValid(PlayerCharacter))
	{
		PlayerLocation = PlayerCharacter->GetActorLocation();
		DistanceToTarget = FVector::Distance(PlayerLocation, EnemyCharacter->GetActorLocation());
	}
}