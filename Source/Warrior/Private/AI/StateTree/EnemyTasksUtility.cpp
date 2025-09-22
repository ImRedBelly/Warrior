// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/StateTree/EnemyTasksUtility.h"

#include "StateTreeExecutionContext.h"
#include "WarriorFunctionLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AI/StateTree/Tasks/STT_GetEnemyTargetInfo.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#pragma region GetPlayerInfo

EStateTreeRunStatus FStateTreeGetEnemyInfoTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.DefaultMaxWalkSpeed = InstanceData.Character->GetCharacterMovement()->MaxWalkSpeed;

	return FStateTreeTaskCommonBase::EnterState(Context, Transition);
}

void FStateTreeGetEnemyInfoTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);


	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

EStateTreeRunStatus FStateTreeGetEnemyInfoTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreeGetEnemyInfoTask::GetDescription(
	const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("Get Enemy Info");
}
#endif

#pragma endregion


#pragma region OrientRotationToTargetActor


EStateTreeRunStatus FStateTreeOrientRotationToTargetActorTask::EnterState(FStateTreeExecutionContext& Context,
                                                                          const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	return FStateTreeTaskCommonBase::EnterState(Context, Transition);
}

void FStateTreeOrientRotationToTargetActorTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

EStateTreeRunStatus FStateTreeOrientRotationToTargetActorTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	APawn* OwnerPawn = InstanceData.OwnerPawn;
	ACharacter* TargetActor = InstanceData.TargetActor;
	float RotationInterpSpeed = InstanceData.RotationInterpSpeed;

	if (OwnerPawn && InstanceData.TargetActor)
	{
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
		const FRotator TargetRotation = FMath::RInterpTo(OwnerPawn->GetActorRotation(), LookAtRotation, DeltaTime, RotationInterpSpeed);

		OwnerPawn->SetActorRotation(TargetRotation);
	}

	return EStateTreeRunStatus::Running;
}


#if WITH_EDITOR
FText FStateTreeOrientRotationToTargetActorTask::GetDescription(
	const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("Orient Rotation To Target Actor"));
}
#endif


#pragma endregion

#pragma region ActiveAbilityByTag


EStateTreeRunStatus FStateTreeActiveAbilityByTagTask::EnterState(FStateTreeExecutionContext& Context,
                                                                 const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwnerPawn && InstanceData.AbilityTagToActivate.IsValid())
	{
		if (UWarriorAbilitySystemComponent* AbilitySystemComponent = UWarriorFunctionLibrary::NativeGetWarriorASCFromActor(InstanceData.OwnerPawn))
		{
			AbilitySystemComponent->TryActivateAbilityByTag(InstanceData.AbilityTagToActivate);
		}
	}

	return EStateTreeRunStatus::Succeeded;
}

void FStateTreeActiveAbilityByTagTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	FStateTreeTaskCommonBase::ExitState(Context, Transition);
}

#if WITH_EDITOR
FText FStateTreeActiveAbilityByTagTask::GetDescription(
	const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup,
	EStateTreeNodeFormatting Formatting) const
{
	if (const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>())
	{
		return FText::FromString(FString::Printf(TEXT("Ability Tag To Active: %s"), *InstanceData->AbilityTagToActivate.ToString()));
	}

	return FText::FromString(TEXT("Ability Tag To Active"));
}
#endif


#pragma endregion
