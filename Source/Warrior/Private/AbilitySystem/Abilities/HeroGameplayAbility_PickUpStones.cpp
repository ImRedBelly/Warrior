// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/HeroGameplayAbility_PickUpStones.h"

#include "Characters/WarriorHeroCharacter.h"
#include "Components/UI/HeroUIComponent.h"
#include "Items/PickUps/WarriorStoneBase.h"
#include "Kismet/KismetSystemLibrary.h"

void UHeroGameplayAbility_PickUpStones::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	GetHeroUIComponentFromActorInfo()->OnStoneInteracted.Broadcast(true);
}

void UHeroGameplayAbility_PickUpStones::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetHeroUIComponentFromActorInfo()->OnStoneInteracted.Broadcast(false);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_PickUpStones::CollectStones()
{
	CollectedStones.Empty();
	TArray<FHitResult> TraceHits;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetHeroCharacterFromActorInfo(),
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		GetHeroCharacterFromActorInfo()->GetActorLocation() + -GetHeroCharacterFromActorInfo()->GetActorUpVector() * BoxTraceDistance,
		BoxTraceSize / 2.f,
		(-GetHeroCharacterFromActorInfo()->GetActorUpVector()).ToOrientationRotator(),
		StonesTraceChannel,
		false,
		TArray<AActor*>(),
		bDrawDebugShape ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		TraceHits,
		true
	);

	for (const FHitResult& TraceHit : TraceHits)
	{
		if (AWarriorStoneBase* FoundStone = Cast<AWarriorStoneBase>(TraceHit.GetActor()))
		{
			CollectedStones.AddUnique(FoundStone);
		}
	}

	if (CollectedStones.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
	}
}

void UHeroGameplayAbility_PickUpStones::ConsumeStones()
{
	if (CollectedStones.IsEmpty())
	{
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		return;
	}

	for (AWarriorStoneBase* CollectedStone : CollectedStones)
	{
		if (CollectedStone)
		{
			CollectedStone->Consume(GetWarriorAbilitySystemComponentFromActorInfo(), GetAbilityLevel());
		}
	}
}
