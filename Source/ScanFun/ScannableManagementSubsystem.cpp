// Attribution: Barbara Bugajak

#include "ScannableManagementSubsystem.h"
#include "QRData.h"
#include "ScannableDataRow.h"
#include "ScannableSubsystemSettings.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "CustomAbilitySystemComponent.h"
#include "ConveyorBelt.h"

void UScannableManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection) {

	Super::Initialize(Collection);

	const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();
	
	check(Settings->QRDataTablePath != nullptr);

	Settings->QRDataTablePath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}


			if (UDataTable* LoadedDataTable = Cast<UDataTable>(LoadedAsset)) {
				QRDataTable = LoadedDataTable;
			}
		}
	));

	ScannableToSpawn_Class = Settings->ScannableToSpawn_Class;
	MinQRScale = Settings->MinQRScale;
	MaxQRScale = Settings->MaxQRScale;

	TagsOfAbilitiesToActivateOnDestructionOfScannable = Settings->TagsOfAbilitiesToActivateOnDestructionOfScannable;

	Settings->RarityDataAssetPath.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}


			if (URarityDataAsset* LoadedDataAsset = Cast<URarityDataAsset>(LoadedAsset)) {
				RarityDataAsset = LoadedDataAsset;

				for (FRarityDataAssetPart& Rarity : RarityDataAsset->Rarities) {
					rarityWeightsSum += Rarity.ProbabilityWeight;
				}
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

	check(QRDataTable != nullptr);
	check(RarityDataAsset != nullptr);

	float RandomValue = FMath::FRandRange(0.0f, rarityWeightsSum);

	FRarityDataAssetPart ChosenRarity;

	float cumulativeProbability = 0.0f;

	for (const FRarityDataAssetPart& Item : RarityDataAsset->Rarities)
	{
		cumulativeProbability += Item.ProbabilityWeight;

		if (RandomValue <= cumulativeProbability)
		{
			ChosenRarity = Item;
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

			Scannables[i]->Destroy();
			Scannables.RemoveAt(i);

			const AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			check(Player);

			UAbilitySystemComponent* ASC =
				UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player, true);
			check(ASC);

			// Get the handle of abilitity to activate
			ASC->TryActivateAbilitiesByTag(TagsOfAbilitiesToActivateOnDestructionOfScannable);
			continue;
		}

		Location.Y -= objectSpeed * DeltaTime;
		Scannables[i]->SetActorLocation(Location);
	}

}

