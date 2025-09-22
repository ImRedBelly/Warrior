// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Perception/AIPerceptionTypes.h"
#include "STT_GetEnemyTargetInfo.generated.h"

class AAIController;

/**
* This is a blueprint task because you need to subscribe to events!
*/

UCLASS(DisplayName="Get Enemy Target Info")
class WARRIOR_API USTT_GetEnemyTargetInfo : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	USTT_GetEnemyTargetInfo(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACharacter> EnemyCharacter;

	UPROPERTY(VisibleAnywhere, Category= Output)
	TObjectPtr<ACharacter> PlayerCharacter;

	UPROPERTY(VisibleAnywhere, Category= Output)
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category= Output)
	float DistanceToTarget = 0.0f;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

private:
	UFUNCTION()
	void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void CalculateValues();
};