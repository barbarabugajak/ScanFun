// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"


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
