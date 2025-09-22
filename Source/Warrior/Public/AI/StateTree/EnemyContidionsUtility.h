// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "EnemyContidionsUtility.generated.h"


#pragma region ShouldAboardAllLogic

USTRUCT()
struct FStateTreeShouldAboardAllLogicConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	AActor* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "Input")
	AActor* TargetActor;

	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget;
};

STATETREE_POD_INSTANCEDATA(FStateTreeShouldAboardAllLogicConditionInstanceData);

USTRUCT(DisplayName = "Should Aboard All Logic")
struct FStateTreeShouldAboardAllLogicCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeShouldAboardAllLogicConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeShouldAboardAllLogicCondition() = default;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

#pragma endregion


#pragma region ComputeSuccessChance


USTRUCT()
struct FStateTreeComputeSuccessChanceConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 0, ClampMax = 1))
	float SuccessChanceMin;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin = 0, ClampMax = 1))
	float SuccessChanceMax;
};

STATETREE_POD_INSTANCEDATA(FStateTreeComputeSuccessChanceConditionInstanceData);

USTRUCT(DisplayName = "Compute Success Chance")
struct FStateTreeComputeSuccessChanceCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeComputeSuccessChanceConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeComputeSuccessChanceCondition() = default;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

#pragma endregion


#pragma region Cooldown


USTRUCT()
struct FStateTreeCooldownConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	AActor* OwningCharacter;

	UPROPERTY(EditDefaultsOnly)
	FName Name;

	UPROPERTY(EditDefaultsOnly)
	float Time;
};

STATETREE_POD_INSTANCEDATA(FStateTreeCooldownConditionInstanceData);

USTRUCT(DisplayName = "Cooldown")
struct FStateTreeCooldownCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCooldownConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeCooldownCondition() = default;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

#pragma endregion

#pragma region DoesActorHaveTag

USTRUCT()
struct FStateTreeDoesActorHaveTagConditionInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AActor> OwnerPawn;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag TagToCheck;

	UPROPERTY(EditDefaultsOnly)
	bool InverseConditionCheck;
};

STATETREE_POD_INSTANCEDATA(FStateTreeDoesActorHaveTagConditionInstanceData);

USTRUCT(DisplayName = "Does Actor Have Tag")
struct FStateTreeDoesActorHaveTagCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeDoesActorHaveTagConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FStateTreeDoesActorHaveTagCondition() = default;

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
	                             EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

#pragma endregion
