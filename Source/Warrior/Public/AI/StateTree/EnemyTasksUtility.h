// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnemyTasksUtility.generated.h"

class AWarriorBaseCharacter;

#pragma region GetEnemyInfo

USTRUCT()
struct FStateTreeGetEnemyInfoInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACharacter> Character;

	UPROPERTY(VisibleAnywhere, Category = Output)
	float DefaultMaxWalkSpeed = 0.0f;
};

USTRUCT(meta=(DisplayName="Get Enemy Info", Category="Combat"))
struct FStateTreeGetEnemyInfoTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeGetEnemyInfoInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};


#pragma endregion

#pragma region OrientRotationToTargetActor

USTRUCT()
struct FStateTreeOrientRotationToTargetActorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACharacter> OwnerPawn;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<ACharacter> TargetActor;

	UPROPERTY(EditDefaultsOnly)
	float RotationInterpSpeed = 0.0f;
};

USTRUCT(meta=(DisplayName="Orient Rotation To Target Actor", Category="Combat"))
struct FStateTreeOrientRotationToTargetActorTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeOrientRotationToTargetActorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};


#pragma endregion

#pragma region ActiveAbilityByTag

USTRUCT()
struct FStateTreeActiveAbilityByTagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ACharacter> OwnerPawn;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AbilityTagToActivate;
	
};

USTRUCT(meta=(DisplayName="Active Ability By Tag", Category="Combat"))
struct FStateTreeActiveAbilityByTagTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FStateTreeActiveAbilityByTagInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};


#pragma endregion
