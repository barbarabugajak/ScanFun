// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"
#include "GE_Score.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"



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
	
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	i += DeltaTime;

	if (i >= 10.f) {
		// Sanity check for Gameplay Effects, Tags & Attributes
		ApplyGameplayEffect_Score(FMath::RandRange(1, 100));
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

	if (!IMC) { UE_LOG(LogTemp, Warning, TEXT("Input Mapping Component not bound")); return; }

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* SubSys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				UE_LOG(LogTemp, Display, TEXT("Enhanced Input Setup"));
				SubSys->AddMappingContext(IMC.LoadSynchronous(), 0);
			}
		}
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (!Input) { UE_LOG(LogTemp, Warning, TEXT("PlayerInputComponent cast to Enhanced Input Component failed")); return; }

	Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);


}

void APlayerCharacter::Move(const FInputActionValue& Value) {

	float InputValue = Value.Get<float>();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Movement: %f"), InputValue));
	}

}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}
