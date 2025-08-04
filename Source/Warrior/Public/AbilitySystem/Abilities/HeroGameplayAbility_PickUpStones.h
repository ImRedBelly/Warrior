// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/WarriorHeroGameplayAbility.h"
#include "HeroGameplayAbility_PickUpStones.generated.h"

class AWarriorStoneBase;
/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroGameplayAbility_PickUpStones : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	UFUNCTION(BlueprintCallable)
	void CollectStones();

	UFUNCTION(BlueprintCallable)
	void ConsumeStones();

private:
	UPROPERTY(EditDefaultsOnly)
	float BoxTraceDistance = 50.f;

	UPROPERTY(EditDefaultsOnly)
	FVector BoxTraceSize = FVector(100.f);

	UPROPERTY(EditDefaultsOnly)
	TArray<TEnumAsByte<EObjectTypeQuery>> StonesTraceChannel;

	UPROPERTY(EditDefaultsOnly)
	bool bDrawDebugShape = false;

	UPROPERTY()
	TArray<AWarriorStoneBase*> CollectedStones;
};
