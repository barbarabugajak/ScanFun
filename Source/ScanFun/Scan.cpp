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
				}

				UScannableManagementSubsystem* ScannableManagementSubsystem = GetWorld()->GetSubsystem<UScannableManagementSubsystem>();

				FRarityDataAssetPart RarityTier = ScannableManagementSubsystem->GetRarityTierOfScannable(Scannable);

				// Rework class to spec
				int AmountOfScanAbilityTags = RarityTier.ScanAbilities.Num();

				if (AmountOfScanAbilityTags <= 0) {
					// Do nothing - no tags, so no abilities
				}
				// There's at least one tag
				else if (AmountOfScanAbilityTags >= 1) {

					TArray<FGameplayAbilitySpec*> AllMatchingSpecs;

					// Get abilities from all tags
					for (int i = 0; i < RarityTier.ScanAbilities.Num(); i++) {
						TArray<FGameplayAbilitySpec*> MatchingAbilities;
						ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(RarityTier.ScanAbilities[i], MatchingAbilities);

						for (int j = 0; j < MatchingAbilities.Num(); j++) {
							// Check if it actually is a ScanAbility [or derived if the designer created a custom child class]
							if (MatchingAbilities[j]->Ability->GetClass()->IsChildOf(UScanAbility::StaticClass())) {
								// No duplicates
								if (!AllMatchingSpecs.Contains(MatchingAbilities[j])) {
									AllMatchingSpecs.Add(MatchingAbilities[j]);
								}
							}
						}
					}

					
					// There's only one ability, check its probability of triggering
					if (AllMatchingSpecs.Num() == 1) {

						UScanAbility* Ability = Cast<UScanAbility>(AllMatchingSpecs[0]->Ability);
						if (!Ability) {
							UE_LOG(LogTemp, Error, TEXT("Ability is invalid"));
							return;
						}

						int randomNumber = FMath::RandRange(0, 100);
						if (randomNumber >= Ability->ChanceOfActivation) {
							ASC->TryActivateAbility(AllMatchingSpecs[0]->Handle);
						}
					}
					// There's more than one ability - trigger one of them at random
					else if (AllMatchingSpecs.Num() > 1) {
						int randomIndex = FMath::RandHelper(AllMatchingSpecs.Num());
						ASC->TryActivateAbility(AllMatchingSpecs[randomIndex]->Handle);
					}
					
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