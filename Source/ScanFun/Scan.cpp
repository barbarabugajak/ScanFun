// Fill out your copyright notice in the Description page of Project Settings.


#include "Scan.h"
#include "PlayerCharacter.h"
#include "Scannable.h"
#include "Engine/OverlapResult.h"


UScan::UScan() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(RequestedTag);
}

void UScan::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) {

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);

	}

	check(ActorInfo != nullptr);
	APlayerCharacter* Character = CastChecked<APlayerCharacter>(ActorInfo->AvatarActor.Get());

	check(Character != nullptr);
	check(Character->GetAbilitySystemComponent() != nullptr);
	UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(Character->GetAbilitySystemComponent());

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	check(Context.IsValid());

	// Get the handles of abilities it is to activate on success
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagsOfAbilitiesToActivateOnSuccess, MatchingGameplayAbilities);

	// Ability itself
	TArray<FOverlapResult> Results;
	bool bAnyOverlaps = GetWorld()->ComponentOverlapMulti(
		Results,
		Character->ScannerConeComp,
		Character->ScannerConeComp->GetComponentLocation(),
		Character->ScannerConeComp->GetComponentRotation()
	);

	if (bAnyOverlaps) {

		for (const FOverlapResult& obj : Results) {
			
			AActor* Actor = obj.GetActor();
			UPrimitiveComponent* Component = obj.GetComponent();

			if (!Actor) 
				continue;
			if (Actor == Character) 
				continue;
			if (!Component) 
				continue;

			AScannable* Scannable = Cast<AScannable>(Actor);

			if (!Scannable)
				continue;

			if (Component->ComponentHasTag("QR")) {
				Scannable->Destroy();
				for (int i = 0; i < MatchingGameplayAbilities.Num(); i++) {
					ASC->TryActivateAbility(MatchingGameplayAbilities[i]->Handle);
				}
				
			}
		}
	}


	EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateInputDirectly, false);
}

bool UScan::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	return true;
}

void UScan::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) {

	if (ScopeLockCount > 0)
	{
		WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UScan::CancelAbility, Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility));
		
		return;
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}