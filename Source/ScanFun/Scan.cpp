// Fill out your copyright notice in the Description page of Project Settings.


#include "Scan.h"
#include "PlayerCharacter.h"
#include "Scannable.h"
#include "Engine/OverlapResult.h"

UScan::UScan() {
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	if (!RequestedTag.IsValid()) return;
	AbilityTags.AddTag(RequestedTag);
}

void UScan::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {

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

	// Ability itself
	TArray<FOverlapResult> Results;
	bool bAnyOverlaps = GetWorld()->ComponentOverlapMulti(
		Results,
		Character->ScannerConeComp,
		Character->ScannerConeComp->GetComponentLocation(),
		Character->ScannerConeComp->GetComponentRotation()
	);

	if (bAnyOverlaps) {

		for (auto& obj : Results) {

			if (!obj.GetActor()) continue;

			if (AScannable* Scannable = Cast<AScannable>(obj.GetActor())) {
				Scannable->Destroy();
				Character->ASC->TryActivateAbilityByClass(Character->GainScore);
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