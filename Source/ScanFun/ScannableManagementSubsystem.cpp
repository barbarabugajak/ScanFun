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
				QRDataTable = LoadedDataTable;
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

	TArray<FString> Rarities = GetRarities();
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

	if (QRDataTable == nullptr) {
		return;
	}
	if (RarityDataAsset == nullptr) {
		return;
	}

	if (!bAScanAbilitiesGranted) {
		HaveScanAbilitiesGranted();
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
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, ChosenRarity.Color, FString::Printf(TEXT("Rarity: %s"), *ChosenRarity.Name));

	TArray< FScannableDataRow*> PossibleItems;

	TArray<FName> RowNames = QRDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		if (FScannableDataRow* Row = QRDataTable->FindRow<FScannableDataRow>(RowName, ""))
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


	Item->Asset.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this, NewScannable, Item](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}
				

			if (UStaticMesh* LoadedMesh = Cast<UStaticMesh>(LoadedAsset)) {
				NewScannable->Mesh->SetStaticMesh(LoadedMesh);
				NewScannable->Mesh->SetRelativeScale3D(FVector(Item->Asset_Scale, Item->Asset_Scale, Item->Asset_Scale));
				FVector SpawnPos = SpawnLocation;
				FVector Origin, Extent;
				NewScannable->GetActorBounds(true, Origin, Extent);
				SpawnPos.Z += Extent.Z;

				NewScannable->SetActorLocation(SpawnPos);
			}
		}
	));

	NewScannable->QR->SetRelativeLocation(Item->QRPosition);
	FRotator NewRot = NewScannable->QR->GetRelativeRotation();
	NewRot.Pitch += FMath::RandRange(0, 360);
	NewScannable->QR->SetRelativeRotation(NewRot);
	double ScaleQR = FMath::FRandRange(MinQRScale, MaxQRScale);
	NewScannable->QR->SetRelativeScale3D(FVector(ScaleQR, ScaleQR, ScaleQR));
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

			const AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			check(Player);

			UAbilitySystemComponent* ASC =
				UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player, true);
			check(ASC);

			// Get the handle of abilitity to activate
			TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
			ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(LoseScoreTagContainer, MatchingGameplayAbilities);

			FGameplayEventData DataSetup;

			DataSetup.OptionalObject = Scannables[i];

			const FGameplayEventData* Data = &DataSetup;

			FGameplayAbilityActorInfo GainScoreActorInfo;
			GainScoreActorInfo.InitFromActor(const_cast<AActor*>(Player), const_cast<AActor*>(Player), ASC);

			for (int j = 0; j < MatchingGameplayAbilities.Num(); j++) {
				ASC->TriggerAbilityFromGameplayEvent(MatchingGameplayAbilities[j]->Handle, &GainScoreActorInfo, ScannableDestroyedEventTag, Data, *ASC);
			}
			
			FRarityDataAssetPart Rarity = GetRarityTierOfScannable(Scannables[i]);

			if (Rarity.FailAbilities.Num() > 0) {
				int index = FMath::RandHelper(Rarity.FailAbilities.Num());
				ASC->TryActivateAbilityByClass(Rarity.FailAbilities[index]);
			}

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

	FString ScannableTierName = Scannable->RarityTierName;

	for (FRarityDataAssetPart Tier : RarityDataAsset->Rarities) {
		if (Tier.Name == ScannableTierName) {
			return Tier;
		}
	}

	return Item;
}

void UScannableManagementSubsystem::UpdateCooldowns() {

	for (TMap<FString, int>::TIterator It = Cooldowns.CreateIterator(); It; ++It){
		if (It->Value <= 0) {
			It.RemoveCurrent();
			continue;
		}
		It->Value -= 1;
	}

}

void UScannableManagementSubsystem::HaveScanAbilitiesGranted() {

	if (!RarityDataAsset) return;

	const AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	check(Player);

	UAbilitySystemComponent* ASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player, true);
	check(ASC);


	for (FRarityDataAssetPart RarityTier : RarityDataAsset->Rarities) {
		for (TSubclassOf<UGameplayAbilityBase> FailAbility : RarityTier.FailAbilities) {
			ASC->GiveAbility(FGameplayAbilitySpec(FailAbility, 1, 0, this));
		}

		for (TSubclassOf<UScanAbility> SuccessAbility : RarityTier.ScanAbilities) {
			ASC->GiveAbility(FGameplayAbilitySpec(SuccessAbility, 1, 0, this));
		}
	}

	bAScanAbilitiesGranted = true;
}


FScannerType UScannableManagementSubsystem::GetScannerTypeFromName(const FString Name) {
	FScannerType Item;

	if (!ScannerDataAsset) return Item;

	for (FScannerType Type : ScannerDataAsset->ScannerTypes) {
		if (Type.Name == Name) {
			return Type;
		}
	}

	return Item;
}