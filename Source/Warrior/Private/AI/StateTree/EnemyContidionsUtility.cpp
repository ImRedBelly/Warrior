// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/StateTree/EnemyContidionsUtility.h"
#include "StateTreeExecutionContext.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystems/GameInstance/CooldownSubsystem.h"


#pragma region ShouldAboardAllLogic

bool FStateTreeShouldAboardAllLogicCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwningCharacter && InstanceData.TargetActor)
	{
		bool IsTargetActorDead = UWarriorFunctionLibrary::NativeDoesActorHaveTag(InstanceData.TargetActor, WarriorGameplayTags::Shared_Status_Death);
		bool IsOwningAIDead = UWarriorFunctionLibrary::NativeDoesActorHaveTag(InstanceData.OwningCharacter, WarriorGameplayTags::Shared_Status_Death);

		if (IsTargetActorDead || IsOwningAIDead || UKismetMathLibrary::NearlyEqual_FloatFloat(InstanceData.DistanceToTarget, 0.f))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
FText FStateTreeShouldAboardAllLogicCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
                                                              const IStateTreeBindingLookup& BindingLookup,
                                                              EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("Should Aboard All Logic>");
}
#endif

#pragma endregion


#pragma region ComputeSuccessChance

bool FStateTreeComputeSuccessChanceCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	float RandomValue = UKismetMathLibrary::RandomFloatInRange(InstanceData.SuccessChanceMin, InstanceData.SuccessChanceMax);
	return UKismetMathLibrary::RandomBoolWithWeight(RandomValue);
}

#if WITH_EDITOR
FText FStateTreeComputeSuccessChanceCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
                                                              const IStateTreeBindingLookup& BindingLookup,
                                                              EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString("Compute Success Chance");
}
#endif

#pragma endregion


#pragma region Cooldown

bool FStateTreeCooldownCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwningCharacter)
	{
		if (UCooldownSubsystem* Subsystem = InstanceData.OwningCharacter->GetGameInstance()->GetSubsystem<UCooldownSubsystem>())
		{
			return Subsystem->HasTimePassed(InstanceData.OwningCharacter, InstanceData.Name, InstanceData.Time);
		}
	}

	return false;
}

#if WITH_EDITOR
FText FStateTreeCooldownCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
                                                  const IStateTreeBindingLookup& BindingLookup,
                                                  EStateTreeNodeFormatting Formatting) const
{
	if (const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>())
	{
		return FText::FromString(FString::Printf(TEXT("Cooldown: %.1fs"), InstanceData->Time));
	}
	
	return FText::FromString("Cooldown");
}
#endif

#pragma endregion


#pragma region DoesActorHaveTag

bool FStateTreeDoesActorHaveTagCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwnerPawn && InstanceData.TagToCheck.IsValid())
	{
		bool Result = UWarriorFunctionLibrary::NativeDoesActorHaveTag(InstanceData.OwnerPawn, InstanceData.TagToCheck);
		return InstanceData.InverseConditionCheck ? !Result : Result;
	}

	return false;
}

#if WITH_EDITOR
FText FStateTreeDoesActorHaveTagCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
                                                  const IStateTreeBindingLookup& BindingLookup,
                                                  EStateTreeNodeFormatting Formatting) const
{
	if (const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>())
	{
		return FText::FromString(FString::Printf(TEXT("Does Actor Have Tag: %s"), *InstanceData->TagToCheck.ToString()));
	}
	
	return FText::FromString("Does Actor Have Tag");
}
#endif

#pragma endregion
