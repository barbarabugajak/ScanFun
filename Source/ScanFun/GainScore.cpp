// Fill out your copyright notice in the Description page of Project Settings.


#include "GainScore.h"
#include "PlayerCharacter.h"

UGainScore::UGainScore() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (!RequestedTag.IsValid()) return;
	AbilityTags.AddTag(RequestedTag);
}

void UGainScore::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {

	if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo)) { UE_LOG(LogTemp, Warning, TEXT("No authority or prediction")); return; }

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}

	APlayerCharacter* Character = CastChecked<APlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character) { UE_LOG(LogTemp, Warning, TEXT("Invalid Avatar")); return; }
	UAbilitySystemComponent* ASC = CastChecked<UAbilitySystemComponent>(Character->GetAbilitySystemComponent());
	if (!Character) { UE_LOG(LogTemp, Warning, TEXT("Invalid Ability System Comp")); return; }

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	if (!Context.IsValid()) { if (!Character) UE_LOG(LogTemp, Warning, TEXT("Invalid Context")); return; }
	if (!Context.IsValid()) { if (!AddScoreEffect_Class) UE_LOG(LogTemp, Warning, TEXT("AddScoreEffect_Class not assigned")); return; }

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddScoreEffect_Class, 1, Context);
	if (!Spec.IsValid()) { if (!AddScoreEffect_Class) UE_LOG(LogTemp, Warning, TEXT("Invalid Spec")); return; }

	if (!ScoreSetByCallerTag.IsValid()) { UE_LOG(LogTemp, Warning, TEXT("Tag for Score is Invalid")); return; }

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