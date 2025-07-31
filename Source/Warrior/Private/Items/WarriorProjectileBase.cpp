// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	CollisionBox->OnComponentHit.AddUniqueDynamic(this, &ThisClass::OnHit);
	CollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnBeginOverlap);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(GetRootComponent());

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->InitialSpeed = 700.f;
	MovementComponent->MaxSpeed = 900.f;
	MovementComponent->Velocity = FVector(1.f, 0.f, 0.f);
	MovementComponent->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.0f;
}

void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (DamagePolicy == EProjectileDamagePolicy::OnBeginOverlap)
	{
		CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void AWarriorProjectileBase::OnHit(UPrimitiveComponent* HitComponent,
                                   AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp,
                                   FVector NormalImpulse,
                                   const FHitResult& Hit)
{
	BP_OnSpawnHitFX(Hit.ImpactPoint);

	APawn* HitPawn = Cast<APawn>(OtherActor);

	if (!HitPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
	{
		Destroy();
		return;
	}

	bool bIsValidBlock = false;
	const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitPawn, WarriorGameplayTags::Player_Status_Blocking);

	if (bIsPlayerBlocking)
	{
		bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(this, HitPawn);
	}

	FGameplayEventData Data;
	Data.Instigator = this;
	Data.Target = HitPawn;

	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitPawn,
			WarriorGameplayTags::Player_Event_SuccessfulBlock,
			Data);
	}
	else
	{
		HandleApplyDamage(HitPawn, Data);
	}

	Destroy();
}

void AWarriorProjectileBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                            AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp,
                                            int32 OtherBodyIndex,
                                            bool bFromSweep,
                                            const FHitResult& SweepResult)
{
	if (OverlapActors.Contains(OtherActor)) return;

	OverlapActors.AddUnique(OtherActor);

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			FGameplayEventData Data;
			Data.Instigator = GetInstigator();
			Data.Target = HitPawn;

			HandleApplyDamage(HitPawn, Data);
		}
	}
}

void AWarriorProjectileBase::HandleApplyDamage(APawn* InHitPawn, const FGameplayEventData& InPayload)
{
	checkf(DamageEffectSpecHandle.IsValid(), TEXT("Forgot to assign a valid spec handle to the projectile: %s"), *GetActorNameOrLabel());

	const bool bWasApplied = UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(GetInstigator(), InHitPawn, DamageEffectSpecHandle);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGameplayTags::Shared_Event_HitReact,
			InPayload);
	}
}
