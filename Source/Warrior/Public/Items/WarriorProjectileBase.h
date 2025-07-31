// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "WarriorProjectileBase.generated.h"

class UProjectileMovementComponent;
class UBoxComponent;
class UNiagaraComponent;
struct FGameplayEventData;

UENUM(BlueprintType)
enum class EProjectileDamagePolicy : uint8
{
	OnHit,
	OnBeginOverlap
};

UCLASS()
class WARRIOR_API AWarriorProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AWarriorProjectileBase();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	UProjectileMovementComponent* MovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Projectile")
	EProjectileDamagePolicy DamagePolicy = EProjectileDamagePolicy::OnHit;

	UPROPERTY(BlueprintReadOnly, Category="Projectile", meta=(ExposeOnSpawn = "true"))
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	                   const FHitResult& Hit);

	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                            bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "On Spawn Hit FX"))
	void BP_OnSpawnHitFX(const FVector& HitLocation);

private:
	void HandleApplyDamage(APawn* InHitPawn, const FGameplayEventData& InPayload);

	TArray<AActor*> OverlapActors;
};
