// Fill out your copyright notice in the Description page of Project Settings.


#include "Scan.h"
#include "PlayerCharacter.h"
#include "ScannableManagementSubsystem.h"
#include "ScanAbility.h"
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
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(GainScoreTag, MatchingGameplayAbilities);

	// Ability itself
	TArray<FOverlapResult> Results;
	bool bAnyOverlaps = GetWorld()->ComponentOverlapMulti(
		Results,
		Character->ScannerConeComp,
		Character->ScannerConeComp->GetComponentLocation(),
		Character->ScannerConeComp->GetComponentRotation()
	);

	bool bWasScanSuccesful = false;

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
			if (Scannable->bWasScanned)
				continue;

			if (Component->ComponentHasTag("QR")) {

				bWasScanSuccesful = true;

				Scannable->bWasScanned = true;

				FGameplayEventData DataSetup;
				
				DataSetup.OptionalObject = Scannable;

				const FGameplayEventData* Data = &DataSetup;
				
				FGameplayAbilityActorInfo* GainScoreActorInfo = const_cast<FGameplayAbilityActorInfo *>(ActorInfo);
				for (int i = 0; i < MatchingGameplayAbilities.Num(); i++) {
					ASC->TriggerAbilityFromGameplayEvent(MatchingGameplayAbilities[i]->Handle, GainScoreActorInfo, GainScoreEventTag, Data, *ASC);
					//ASC->TryActivateAbility(MatchingGameplayAbilities[i]->Handle);
				}

				UScannableManagementSubsystem* ScannableManagementSubsystem = GetWorld()->GetSubsystem<UScannableManagementSubsystem>();

				FRarityDataAssetPart RarityTier = ScannableManagementSubsystem->GetRarityTierOfScannable(Scannable);
				UScanAbility* Ability = nullptr; // Invalid if not Case 1
				int randInt;
				switch (RarityTier.ScanAbilities.Num()) {
					case 0:
						break; // No abilities
					case 1: 
						randInt = FMath::RandRange(0, 100);
						Ability = RarityTier.ScanAbilities[0]->GetDefaultObject<UScanAbility>();
						if (!Ability) {
							UE_LOG(LogTemp, Error, TEXT("Ability is invalid"));
							return;
						}
						if (randInt >= Ability->ChanceOfActivation) {
							ASC->TryActivateAbilityByClass(RarityTier.ScanAbilities[0]);
						}
						break;
					default:
						randInt = FMath::RandRange(0, RarityTier.ScanAbilities.Num()-1);
						ASC->TryActivateAbilityByClass(RarityTier.ScanAbilities[randInt]);
						break;
				}
				
			}
		}
	}
	

	Character->ScanAbility_Activated(bWasScanSuccesful);

	// Cooldown
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CooldownGE->GetClass(), 1, Context);
		Spec.Data->SetSetByCallerMagnitude(CooldownMagnitudeTag, Character->CurrentScanerType.Cooldown);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
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