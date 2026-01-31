// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystemGlobals.h"
#include "GE_Score.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "ScannableManagementSubsystem.h"
#include "ScannableSubsystemSettings.h"
#include "Scan.h"
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
	SetupTagListeners();
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	checkf(ASC != nullptr, TEXT("ASC is null in BeginPlay"));
	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &APlayerCharacter::OnActiveGameplayEffectAddedCallback);

	UScannableManagementSubsystem* ScannableSubSys = GetWorld()->GetSubsystem< UScannableManagementSubsystem>();
	ScannableSubSys->Player = this;
	ScannableSubSys->ASC = ASC;

	ASC->GetGameplayAttributeValueChangeDelegate(BasicDataAttributeSet->GetScoreAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			AttributeSet_ScoreChaned(Data.OldValue, Data.NewValue);
		}
	);

	ASC->GetGameplayAttributeValueChangeDelegate(BasicDataAttributeSet->GetWorldColorSaturationAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			if (!GetWorld()) return; // No world initialized yet

			APostProcessVolume* PostProcess = Cast<APostProcessVolume>(
				UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass()));

			if (IsValid(PostProcess)) {
				PostProcess->Settings.ColorSaturation = FVector4(1, 1, 1, Data.NewValue);
			}
		}
	);

	ASC->AbilityFailedCallbacks.AddLambda(
		[this](const UGameplayAbility* Ability, const FGameplayTagContainer& FailureTags)
		{
			const UScan* Scan = Cast<UScan>(Ability);
			if (Scan) {
				ScanAbility_ActivationFailed();
			}
		}
	);


	// Grant Gameplay Abilities
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<UGameplayAbilityBase> AbilityClass : GrantedAbilities) {
		ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0, this));
	}
	ScannableSubSys->bAScanAbilitiesGranted = true;
	DynMaterial = ScannerConeComp->CreateDynamicMaterialInstance(0);
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector RelativeScale = CurrentScanerType.MeshScale;
	RelativeScale.Y *= currentBeamWidthCoefficient;
	ScannerConeComp->SetRelativeScale3D(RelativeScale);

}

// Only works for Duration or Infinite effects, NOT instant
void APlayerCharacter::OnActiveGameplayEffectAddedCallback(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle) {
	
	if (!SpecApplied.Def) return;
	
	if (SpecApplied.Def.GetClass() == GE_Cooldown_Class) {
		
		UWorld* World = GetWorld();

		if (!World) {
			return;
		}	
		UScannableManagementSubsystem* ScannableSubSys = World->GetSubsystem< UScannableManagementSubsystem>();
		if (!ScannableSubSys) {
			return;
		}

		// Associate the Active GameplayEffect with Scanner Type
		ScannableSubSys->ScannerCooldowns.FindOrAdd(CurrentScanerType.Name) = ActiveHandle;

		// Add removed delegate 
		ASC->OnGameplayEffectRemoved_InfoDelegate(ActiveHandle)->AddLambda(
			[this](const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo)
			{
				UScannableManagementSubsystem* ScannableSubSys = GetWorld()->GetSubsystem< UScannableManagementSubsystem>();
				if (!ScannableSubSys) return;

				// As it is the Scanner Type that is the key, it needs to be found first. O(n) is acceptable here
				if (const FName* Key = ScannableSubSys->ScannerCooldowns.FindKey(GameplayEffectRemovalInfo.ActiveEffect->Handle))
				{
					ScannableSubSys->ScannerCooldowns.FindAndRemoveChecked(*Key);
				}
			}
		);
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

void APlayerCharacter::SetupTagListeners()
{
	if (!ASC) return;

	ASC->RegisterGameplayTagEvent(BeamWidenTag, EGameplayTagEventType::NewOrRemoved).AddLambda(
		[this](const FGameplayTag Tag, int32 NewCount) {
			// Added
			if (NewCount > 0) {
				ScanAbility_WidenBeam(currentBeamWidthCoefficient, targetBeamWidthCoefficient);
			}
			// Removed
			else {
				ScanAbility_WidenBeam(currentBeamWidthCoefficient, 1.0f);
			}
		}
	);

	ASC->RegisterGameplayTagEvent(JitterConveyorTag, EGameplayTagEventType::NewOrRemoved).AddLambda(
		[this](const FGameplayTag Tag, int32 NewCount) {

			if (!GetWorld()) {
				return;
			}

			UScannableManagementSubsystem* SubSys = GetWorld()->GetSubsystem<UScannableManagementSubsystem>();

			// Added
			if (NewCount > 0) {
				SubSys->bIsJittering = true;
			}
			// Removed
			else {
				SubSys->bIsJittering = false;
				SubSys->jitterCoefficient = 1.0f;
			}
		}
	);
}

void APlayerCharacter::SetupScannerBeamParams(FScannerType ScannerType) {

	DynMaterial = ScannerConeComp->CreateDynamicMaterialInstance(0);

	UScannableManagementSubsystem* ScannableSubSys = GetWorld()->GetSubsystem< UScannableManagementSubsystem>();
	FLinearColor Color = ScannableSubSys->GetColorOfScanner(ScannerType);
	DynMaterial->SetVectorParameterValue(TEXT("Color"), Color);

	ScannerConeComp->SetStaticMesh(ScannerType.ScannerMesh);

	FVector RelativeScale = ScannerType.MeshScale;
	RelativeScale.Y *= currentBeamWidthCoefficient;
	ScannerConeComp->SetRelativeScale3D(RelativeScale);

	FVector BoxExtent = ScannerConeComp->Bounds.BoxExtent;
	FVector CurrentLocation = ScannerConeComp->GetComponentLocation();

	FVector Position(0, 0, -1.9*BoxExtent.Z);

	ScannerConeComp->SetRelativeLocation(Position);

	CurrentScanerType = ScannerType;
}