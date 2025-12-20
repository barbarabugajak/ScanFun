// Attribution: Barbara Bugajak


#include "ScannableManagementSubsystem.h"
#include "QRData.h"
#include "ScannableSubsystemSettings.h"

void UScannableManagementSubsystem::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (spawnDelayValue > 10.f) {
		SpawnScannable();
		spawnDelayValue = 0.f;
	}

}

void UScannableManagementSubsystem::SpawnScannable() {

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("Spawn a Scannable"));

	AScannable* NewScannable = Cast<AScannable>(GetWorld()->SpawnActor(ScannableToSpawn_Class));

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

void UScannableManagementSubsystem::UpdateScannables() {

}

void UScannableManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection) {

	Super::Initialize(Collection);

	const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();

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