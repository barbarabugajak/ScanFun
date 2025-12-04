// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"
#include "GE_Score.h"


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
	IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals()->GetAttributeSetInitter()->InitAttributeSetDefaults(ASC, "PlayerCharacter", 1, true);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
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
	if (Context.IsValid() && UGE_Score_Class) {
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_Score_Class, 1, Context);
		if (Spec.IsValid()) {
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Player.Effect.Score"), value);

			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			UE_LOG(LogTemp, Log, TEXT("Score++"));
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

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}
