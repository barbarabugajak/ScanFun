// Attribution: Barbara Bugajak


#include "ScannableManagementSubsystem.h"
#include "QRData.h"
#include "ScannableSubsystemSettings.h"
#include "Kismet/GameplayStatics.h"
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

}

void UScannableManagementSubsystem::SpawnScannable() {

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("Spawned a Scannable"));

	AScannable* NewScannable = GetWorld()->SpawnActor<AScannable>(
		ScannableToSpawn_Class,
		SpawnLocation,
		FRotator::ZeroRotator
	);

	

	Scannables.Add(NewScannable);

	if (!QRDataTable)
		return;

	TArray<FName> RowNames = QRDataTable->GetRowNames();

	FQRData* Item = QRDataTable->FindRow<FQRData>(RowNames[FMath::RandRange(0, RowNames.Num() - 1)], "");
	
	if (Item->Asset.IsNull()) {
		return;
	}


	Item->Asset.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this, NewScannable](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}
				

			if (UStaticMesh* LoadedMesh = Cast<UStaticMesh>(LoadedAsset)) {
				NewScannable->Mesh->SetStaticMesh(LoadedMesh);

				FVector SpawnPos = SpawnLocation;
				FVector Origin, Extent;
				NewScannable->GetActorBounds(true, Origin, Extent);
				SpawnPos.Z += Extent.Z;

				DrawDebugSphere(GetWorld(), SpawnPos, 10.0f, 100.0f, FColor::Yellow, true);
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

		// Basic movement for testing purposes, not production ready-
		/*FVector Location = Scannables[i]->GetActorLocation();
		Location.Y += objectSpeed * DeltaTime;
		Scannables[i]->SetActorLocation(Location);*/
	}

}

