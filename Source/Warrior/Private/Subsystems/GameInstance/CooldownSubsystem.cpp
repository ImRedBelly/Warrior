// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/GameInstance/CooldownSubsystem.h"

bool UCooldownSubsystem::HasTimePassed(const AActor* Actor, FName Key, float Interval)
{
	if (!Actor || !IsValid(Actor) || Actor->IsPendingKillPending())
	{
		return false;
	}

	if (Interval < 0.f)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Invalid cooldown interval: %f"), Interval);
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();

	TWeakObjectPtr WeakActor = Actor;

	if (!TimeMap.Contains(WeakActor))
	{
		const_cast<AActor*>(Actor)->OnDestroyed.AddDynamic(this, &UCooldownSubsystem::OnActorDestroyed);
	}

	TMap<FName, float>& ActorMap = TimeMap.FindOrAdd(WeakActor);

	float* LastTime = ActorMap.Find(Key);
	if (LastTime)
	{
		if (CurrentTime - *LastTime >= Interval)
		{
			*LastTime = CurrentTime;
			return true;
		}
		return false;
	}

	ActorMap.Add(Key, CurrentTime);
	return true;
}


void UCooldownSubsystem::OnActorDestroyed(AActor* DestroyedActor)
{
	if (!DestroyedActor)
	{
		return;
	}

	TimeMap.Remove(DestroyedActor);
}