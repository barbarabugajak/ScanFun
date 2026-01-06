// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"
#include "GE_Score.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "ScannableManagementSubsystem.h"
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
	checkf(ASC != nullptr, TEXT("Ability System Component did not initialize properly"));
	ASC->InitAbilityActorInfo(this, this);
	IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals()->GetAttributeSetInitter()->InitAttributeSetDefaults(ASC, "PlayerCharacter", /*Level=*/1, /*IsInitialLoad=*/true);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	checkf(ASC != nullptr, TEXT("ASC is null in BeginPlay"));
	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &APlayerCharacter::OnActiveGameplayEffectAddedCallback);

	ASC->GetGameplayAttributeValueChangeDelegate(BasicDataAttributeSet->GetScoreAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			AttributeSet_ScoreChaned(Data.OldValue, Data.NewValue);
		}
	);


	// Grant Gameplay Abilities
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<UGameplayAbilityBase> AbilityClass : StartingAbilities) {
		ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0, this));
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Only works for Duration or Infinite effects, NOT instant
void APlayerCharacter::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle) {
	
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

void APlayerCharacter::SetupScannerConeParams(FScannerType ScannerType) {

	DynMaterial = ScannerConeComp->CreateDynamicMaterialInstance(0);

	DynMaterial->SetVectorParameterValue(TEXT("Color"), ScannerType.Color);

	ScannerConeComp->SetStaticMesh(ScannerType.ScannerMesh);
	ScannerConeComp->SetRelativeScale3D(ScannerType.MeshScale);

	FVector BoxExtent = ScannerConeComp->Bounds.BoxExtent;
	FVector CurrentLocation = ScannerConeComp->GetComponentLocation();

	FVector Position(0, 0, -1.9*BoxExtent.Z);

	ScannerConeComp->SetRelativeLocation(Position);
}