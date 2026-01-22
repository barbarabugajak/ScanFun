// Attribution: Barbara Bugajak

#include "ScannableManagementSubsystem.h"
#include "QRData.h"
#include "ScannableDataRow.h"
#include "ScannableSubsystemSettings.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "CustomAbilitySystemComponent.h"
#include "ScanAbility.h"
#include "ConveyorBelt.h"
#include "PlayerCharacter.h"

void UScannableManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection) {

	Super::Initialize(Collection);

	const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();
	
	check(Settings->QRDataTablePath != nullptr);

	Settings->QRDataTablePath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {

			if (!LoadedAsset) {
				UE_LOG(LogTemp, Error, TEXT("Asset did not load from path: "), *Path.ToString());
			}

			if (UDataTable* LoadedDataTable = Cast<UDataTable>(LoadedAsset)) {
				ScannablesData = LoadedDataTable;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Loaded asset did not match type. Loaded from %s"), *Path.ToString());
			}
		}
	));

	ScannableToSpawn_Class = Settings->ScannableToSpawn_Class;
	MinQRScale = Settings->MinQRScale;
	MaxQRScale = Settings->MaxQRScale;
	ScannableDestroyedEventTag = Settings->ScannableDestroyedEventTag;
	spawnDelay = Settings->SpawnDelay;
	LoseScoreTagContainer = Settings->LoseScoreTagContainer;
	objectSpeed = Settings->ConveyorBeltSpeed;


	Settings->RarityDataAssetPath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			
			if (!LoadedAsset) {
				UE_LOG(LogTemp, Error, TEXT("Asset did not load from path: "), *Path.ToString());
			}

			if (URarityDataAsset* LoadedDataAsset = Cast<URarityDataAsset>(LoadedAsset)) {

				RarityDataAsset = LoadedDataAsset;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Loaded asset did not match type. Loaded from %s"), *Path.ToString());
			}
		}
	));

	Settings->ScannerDataAssetPath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {

			if (!LoadedAsset) {
				UE_LOG(LogTemp, Error, TEXT("Asset did not load from path: "), *Path.ToString());
			}

			if (UScannerData* LoadedDataAsset = Cast<UScannerData>(LoadedAsset)) {

				ScannerDataAsset = LoadedDataAsset;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Loaded asset did not match type. Loaded from %s"), *Path.ToString());
			}
		}
	));

	Settings->QRCodeTypesDataAssetPath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {

			if (!LoadedAsset) {
				UE_LOG(LogTemp, Error, TEXT("Asset did not load from path: "), *Path.ToString());
			}

			if (UQRCodeType* LoadedDataAsset = Cast<UQRCodeType>(LoadedAsset)) {

				QRCodeTypeDataAsset = LoadedDataAsset;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Loaded asset did not match type. Loaded from %s"), *Path.ToString());
			}
		}
	));

}

TStatId UScannableManagementSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UScannableManagementSubsystem, STATGROUP_Tickables);
}


void UScannableManagementSubsystem::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	spawnDelayValueCounter += DeltaTime;

	if (spawnDelayValueCounter >= spawnDelay) {
		SpawnScannable();
		spawnDelayValueCounter = 0.f;
	}

	UpdateScannables(DeltaTime);
\
	if (!bWasInitialScannerBeamSetup) {
		SetupInitialScannerBeam();
	}
}

void UScannableManagementSubsystem::SetConveyorBeltSetupRelatedVariables(AConveyorBelt* ConveyorBelt) {

	check(ConveyorBelt != nullptr);

	Belt = ConveyorBelt;

	FVector BeltLocation = Belt->GetActorLocation();
	FVector BeltOrigin, BeltExtent;
	ConveyorBelt->GetActorBounds(true, BeltOrigin, BeltExtent);
	
	SpawnLocation = BeltLocation + BeltExtent;
	SpawnLocation.X = BeltLocation.X;
	DestructionLocation = BeltLocation - BeltExtent;
	DestructionLocation.Z = SpawnLocation.Z;
}

void UScannableManagementSubsystem::SpawnScannable() {

	if (ScannablesData == nullptr) {
		return;
	}
	if (RarityDataAsset == nullptr) {
		return;
	}
	if (!bAScanAbilitiesGranted) {
		return;
	}

	UpdateCooldowns();

	TArray<FRarityDataAssetPart> PossibleRarities = RarityDataAsset->Rarities;

	for (int i = PossibleRarities.Num() - 1; i >= 0; i--) {
		if (Cooldowns.Find(PossibleRarities[i].Name)) {
			PossibleRarities.RemoveAt(i);
		}
	}


	if (PossibleRarities.Num() == 0) {
		return; // No Spawn if no tier possible
	}

	int rarityWeightsSum = 0;

	for (FRarityDataAssetPart& Rarity : PossibleRarities) {
		rarityWeightsSum += Rarity.ProbabilityWeight;
	}

	float RandomValue = FMath::FRandRange(0.0f, rarityWeightsSum);

	FRarityDataAssetPart ChosenRarity;

	float cumulativeProbability = 0.0f;

	for (const FRarityDataAssetPart& Item : PossibleRarities)
	{	
		cumulativeProbability += Item.ProbabilityWeight;

		if (RandomValue <= cumulativeProbability)
		{
			ChosenRarity = Item;
			Cooldowns.FindOrAdd(ChosenRarity.Name) = ChosenRarity.Cooldown;
			break; 
		}
	}
	
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, ChosenRarity.Color, FString::Printf(TEXT("Rarity: %s"), *ChosenRarity.Name.ToString()));

	TArray< FScannableDataRow*> PossibleItems;

	TArray<FName> RowNames = ScannablesData->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		if (FScannableDataRow* Row = ScannablesData->FindRow<FScannableDataRow>(RowName, ""))
		{
			if (Row->Rarity == ChosenRarity.Name)
			{
				PossibleItems.Add(Row);
			}
		}
	}

	FScannableDataRow* Item = PossibleItems[FMath::RandRange(0, PossibleItems.Num()-1)];
	
	check(!Item->Asset.IsNull());

	AScannable* NewScannable = GetWorld()->SpawnActor<AScannable>(
		ScannableToSpawn_Class,
		SpawnLocation,
		FRotator::ZeroRotator
	);
	NewScannable->RarityTierName = ChosenRarity.Name;
	Scannables.Add(NewScannable);

	TArray<FQRCodeTypeEntry> PossibleQRCodeTypes = GetQRCodeTypesOfRarityType(ChosenRarity);

	checkf(PossibleQRCodeTypes.Num() > 0,TEXT("Every Rarity Tier must have at least one QR Code Type, but the %s tier has none."), *ChosenRarity.Name.ToString());

	FQRCodeTypeEntry ChosenQR = PossibleQRCodeTypes[FMath::RandHelper(PossibleQRCodeTypes.Num())];

	Item->Asset.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this, NewScannable, Item, ChosenQR](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}
				

			if (UStaticMesh* LoadedMesh = Cast<UStaticMesh>(LoadedAsset)) {

				GetWorld()->GetTimerManager().SetTimerForNextTick([this, NewScannable, LoadedMesh, Item, ChosenQR]()
					{
					NewScannable->Mesh->SetStaticMesh(LoadedMesh);
					NewScannable->Mesh->SetRelativeScale3D(FVector(Item->Asset_Scale, Item->Asset_Scale, Item->Asset_Scale));
					FVector SpawnPos = SpawnLocation;
					FVector Origin, Extent;
					NewScannable->GetActorBounds(true, Origin, Extent);
					SpawnPos.Z += Extent.Z;

					NewScannable->SetActorLocation(SpawnPos);

					NewScannable->SetupQRCode(*Item, ChosenQR);
				});
			}
		}
	));
}

void UScannableManagementSubsystem::UpdateScannables(float DeltaTime) {

	for (int i = Scannables.Num() - 1; i >= 0; i--)
	{
		if (Scannables[i] && Scannables[i]->bWasScanned)
		{			
			Scannables[i]->Destroy();
			Scannables.RemoveAt(i);

			continue;
		}

		FVector Location = Scannables[i]->GetActorLocation();

		if (Location.Y < DestructionLocation.Y) {
			TriggerLoseScore(i);
			TryTriggeringRandomFailAbility(i);
			

			Scannables[i]->Destroy();
			Scannables.RemoveAt(i);
			continue;
		}

		Location.Y -= objectSpeed * DeltaTime;
		Scannables[i]->SetActorLocation(Location);
	}

}

FRarityDataAssetPart UScannableManagementSubsystem::GetRarityTierOfScannable(const AScannable* Scannable) {
	
	FRarityDataAssetPart Item;
	
	if (!Scannable) {
		return Item;
	}

	FName ScannableTierName = Scannable->RarityTierName;

	for (FRarityDataAssetPart Tier : RarityDataAsset->Rarities) {
		if (Tier.Name == ScannableTierName) {
			return Tier;
		}
	}

	return Item;
}

void UScannableManagementSubsystem::UpdateCooldowns() {

	for (TMap<FName, int>::TIterator It = Cooldowns.CreateIterator(); It; ++It){
		if (It->Value <= 0) {
			It.RemoveCurrent();
			continue;
		}
		It->Value -= 1;
	}

}


FScannerType UScannableManagementSubsystem::GetScannerTypeFromName(const FName Name) {
	FScannerType Item;

	if (!ScannerDataAsset) return Item;

	for (FScannerType Type : ScannerDataAsset->ScannerTypes) {
		if (Type.Name == Name) {
			return Type;
		}
	}

	return Item;
}

void UScannableManagementSubsystem::TriggerLoseScore(int indexOfScannable) {

	if (!(ASC)) {
		UE_LOG(LogTemp, Error, TEXT("ASC ref invalid in ScannableManagementSubsystem"));
		return;
	}

	if (!(Player)) {
		UE_LOG(LogTemp, Error, TEXT("Player ref invalid in ScannableManagementSubsystem"));
		return;
	}

	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(LoseScoreTagContainer, MatchingGameplayAbilities);

	FGameplayEventData DataSetup;
	DataSetup.OptionalObject = Scannables[indexOfScannable];

	const FGameplayEventData* Data = &DataSetup;
	FGameplayAbilityActorInfo GainScoreActorInfo;
	GainScoreActorInfo.InitFromActor(const_cast<AActor*>(Player), const_cast<AActor*>(Player), ASC);

	// This assmues there might be more than one Lose Score Ability, which is OK, assuming that's what the designer wants
	// There will usually be only one
	for (int j = 0; j < MatchingGameplayAbilities.Num(); j++) {
		ASC->TriggerAbilityFromGameplayEvent(MatchingGameplayAbilities[j]->Handle, &GainScoreActorInfo, ScannableDestroyedEventTag, Data, *ASC);
	}

}

void UScannableManagementSubsystem::TryTriggeringRandomFailAbility(int indexOfScannable) {

	if (!(ASC)) {
		UE_LOG(LogTemp, Error, TEXT("ASC ref invalid in ScannableManagementSubsystem"));
		return;
	}

	if (!(Player)) {
		UE_LOG(LogTemp, Error, TEXT("Player ref invalid in ScannableManagementSubsystem"));
		return;
	}

	// Abilities can duplicate, remember! 
	FRarityDataAssetPart Rarity = GetRarityTierOfScannable(Scannables[indexOfScannable]);
		
	TArray < FGameplayAbilitySpec* > AllMatchingSpecs;

	for (int i = 0; i < Rarity.FailAbilities.Num(); i++){
		TArray<FGameplayAbilitySpec*> MatchingAbilities;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(Rarity.FailAbilities[i], MatchingAbilities);
		AllMatchingSpecs.Append(MatchingAbilities);
	}
	
	if (AllMatchingSpecs.Num() <= 0) {
		return; // No ability to activate
	}

	int index = FMath::RandHelper(AllMatchingSpecs.Num());
	ASC->TryActivateAbility(AllMatchingSpecs[index]->Handle);
}

void UScannableManagementSubsystem::SetupInitialScannerBeam() {
	if (!ScannerDataAsset) {
		return; 
	}

	checkf(ScannerDataAsset->ScannerTypes.Num() > 0, TEXT("There are no Scanners in Scanner Data Asset. At least one must be added in Editor"));

	if (!Player) {
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Player);
	PlayerCharacter->SetupScannerBeamParams(ScannerDataAsset->ScannerTypes[0]); // We assume 0 as default

	bWasInitialScannerBeamSetup = true;
}

FQRCodeTypeEntry UScannableManagementSubsystem::GetQRCodeTypeFromName(const FName Name) {

	FQRCodeTypeEntry Type;

	if (!QRCodeTypeDataAsset) {
		return Type;
	}

	for (FQRCodeTypeEntry Entry : QRCodeTypeDataAsset->Entries) {
		if (Entry.Name == Name) {
			return Entry;
		}
	}

	return Type;
}

// Helper to get all possible QR Code Types from Rarity 
TArray<FQRCodeTypeEntry> UScannableManagementSubsystem::GetQRCodeTypesOfRarityType(const FRarityDataAssetPart RarityTier) {
	
	TArray<FQRCodeTypeEntry> Types;

	if (!RarityDataAsset) {
		return Types;
	}

	if (!QRCodeTypeDataAsset) {
		return Types;
	}

	if (!QRCodeTypeDataAsset->DefaultQRCode.IsNone()) {
		Types.Add(GetQRCodeTypeFromName(QRCodeTypeDataAsset->DefaultQRCode));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No Default QR Code Assigned"))
	}

	for (FName Name : RarityTier.QRCodeTypes) {
		FQRCodeTypeEntry Entry = GetQRCodeTypeFromName(Name);
		if (!Entry.Name.IsNone()) {
			Types.Add(GetQRCodeTypeFromName(Name));
		}
	}

	return Types;
}

FLinearColor UScannableManagementSubsystem::GetColorOfScanner(const FScannerType Scanner) {
	FQRCodeTypeEntry QRCode = GetQRCodeTypeFromName(Scanner.AssignedQRCode);
	
	if (!QRCodeTypeDataAsset) {
		UE_LOG(LogTemp, Error, TEXT("QR Code Types Data Asset has not been loaded yet"));
	}

	if (QRCode.Name.IsNone()) {
		UE_LOG(LogTemp, Warning, TEXT("%s has no QR code assigned. Color set to default"), *Scanner.Name.ToString());
		return GetQRCodeTypeFromName(QRCodeTypeDataAsset->DefaultQRCode).Color; // Retrieving color of default QR, CDO's color should it fail
	}
	
	return QRCode.Color;
}