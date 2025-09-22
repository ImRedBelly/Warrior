// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CooldownSubsystem.generated.h"

/**
* 
*/

UCLASS()
class WARRIOR_API UCooldownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool HasTimePassed(const AActor* Actor, FName Key, float Interval);

private:
	TMap<TWeakObjectPtr<const AActor>, TMap<FName, float>> TimeMap;

	UFUNCTION()
	void OnActorDestroyed(AActor* DestroyedActor);
};