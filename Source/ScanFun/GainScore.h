// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"
#include "GainScore.generated.h"

/**
 * 
 */
UCLASS()
class SCANFUN_API UGainScore : public UGameplayAbilityBase
{
	GENERATED_BODY()

	public: 
		UGainScore();

		virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
		virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;
		virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties | GameplayEffects ")
		TSubclassOf<UGameplayEffect> AddScoreEffect_Class;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties | GameplayTags")
		FGameplayTag RequestedTag;

		UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Properties | GameplayTags")
		FGameplayTag ScoreSetByCallerTag;

	
};
