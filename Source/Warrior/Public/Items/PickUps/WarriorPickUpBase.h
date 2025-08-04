// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WarriorPickUpBase.generated.h"

class USphereComponent;

UCLASS()
class WARRIOR_API AWarriorPickUpBase : public AActor
{
	GENERATED_BODY()

public:
	AWarriorPickUpBase();

protected:
#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pick Up Interaction")
	USphereComponent* PickUpCollisionSphere;

#pragma endregion

	UFUNCTION()
	virtual void OnPickUpCollisionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
