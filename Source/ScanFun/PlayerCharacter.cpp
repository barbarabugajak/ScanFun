// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"
#include "GE_Score.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ASC = CreateDefaultSubobject<UCustomAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	BasicDataAttributeSet = CreateDefaultSubobject<UPlayerBasicAttributeSet>(TEXT("Basic Data Attribute Set"));
	ScannerConeComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Scanner Cone Component"));
	ScannerConeComp->SetupAttachment(RootComponent);
}

void APlayerCharacter::PostInitializeComponents() {

	Super::PostInitializeComponents();
	ASC->InitAbilityActorInfo(this, this);
	IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals()->GetAttributeSetInitter()->InitAttributeSetDefaults(ASC, "PlayerCharacter", /*Level=*/1, /*IsInitialLoad=*/true);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (ASC) {
		ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &APlayerCharacter::OnActiveGameplayEffectAddedCallback);
		UE_LOG(LogTemp, Warning, TEXT("Bound OnActiveGameplayEffectAddedDelegateToSelf on %s"), *GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("ASC is null in BeginPlay for %s"), *GetName());
	}
	
	// Grant Gameplay Abilities
	if (GetLocalRole() != ROLE_Authority || !ASC)
	{
		return;
	}

	ASC->GiveAbility(FGameplayAbilitySpec(GainScore, 1, 0, this));
	ASC->GiveAbility(FGameplayAbilitySpec(Scan, 1, 0, this));
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Only works for Duration or Infinite effects, NOT instant
void APlayerCharacter::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle) {
	
    UE_LOG(LogTemp, Warning, TEXT("Effect added: %s"), *SpecApplied.Def->GetName());

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}