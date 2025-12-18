// Fill out your copyright notice in the Description page of Project Settings.


#include "GainScore.h"
#include "PlayerCharacter.h"

UGainScore::UGainScore() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(RequestedTag);
}

void UGainScore::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	}

	check(ActorInfo != nullptr);
	APlayerCharacter* Character = CastChecked<APlayerCharacter>(ActorInfo->AvatarActor.Get());

	check(Character != nullptr);
	check(Character->GetAbilitySystemComponent() != nullptr);
	UAbilitySystemComponent* ASC = CastChecked<UAbilitySystemComponent>(Character->GetAbilitySystemComponent());

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	check(Context.IsValid());

	checkf(AddScoreEffect_Class != nullptr, TEXT("AddScoreEffect_Class not assigned"));
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddScoreEffect_Class, 1, Context);

	check(Spec.IsValid());
	checkf(ScoreSetByCallerTag.IsValid(), TEXT("Tag for Score is Invalid"));


	// Choose a random Number, for sanity check
	int value = FMath::RandRange(1, 100);
	Spec.Data->SetSetByCallerMagnitude(ScoreSetByCallerTag, value);
	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	
	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateInputDirectly, false);
}

bool UGainScore::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	return true;
}

void UGainScore::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) {
	
	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UGainScore::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}