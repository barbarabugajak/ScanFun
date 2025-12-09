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
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	i += DeltaTime;

	// Sanity check for Gameplay Effects, Tags & Attributes
	if (i >= 10.f) {
		ASC->TryActivateAbilityByClass(GainScore);
		i = 0;
	}

}

void APlayerCharacter::ApplyGameplayEffect_Score(float value) {

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	if (Context.IsValid() && AddScoreEffect_Class) {
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AddScoreEffect_Class, 1, Context);
		if (Spec.IsValid()) {

			if (!ScoreSetByCallerTag.IsValid()) UE_LOG(LogTemp, Warning, TEXT("Tag for Score is Invalid"));

			Spec.Data->SetSetByCallerMagnitude(ScoreSetByCallerTag, value);
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			i = 0.f;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Spec is invalid"));
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Context is invalid"));
	}
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

bool APlayerCharacter::IsInViewport() {

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return false;

	FVector2D ScreenLocation;
	PC->ProjectWorldLocationToScreen(GetActorLocation(), ScreenLocation);

	int32 viewportX;
	int32 viewportY;
	PC->GetViewportSize(viewportX, viewportY);

	if (ScreenLocation.X < 0 || ScreenLocation.X > viewportX) return false;
	if (ScreenLocation.Y < 0 || ScreenLocation.Y > viewportY) return false;

	/*if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-3, 0.1f, FColor::Yellow, TEXT("Visible"));
	}*/

	return true;
}
