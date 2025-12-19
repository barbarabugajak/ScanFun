// Fill out your copyright notice in the Description page of Project Settings.


#include "Scannable.h"
#include "QRData.h"

// Sets default values
AScannable::AScannable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	QR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QR"));
	QR->SetupAttachment(Mesh);
	
}

// Called when the game starts or when spawned
void AScannable::BeginPlay()
{
	Super::BeginPlay();

	SetupQRCode();

}

// Called every frame
void AScannable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AScannable::SetupQRCode() {

	if (!QRDataTable)
		return;

	TArray<FName> RowNames = QRDataTable->GetRowNames();

	FQRData* Item = QRDataTable->FindRow<FQRData>(RowNames[FMath::RandRange(0, RowNames.Num() - 1)], "");
	
	if (Item->Asset.IsNull()) {
		return;
	}

	Item->Asset.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateLambda(
		[this](const FSoftObjectPath& Path, UObject* LoadedAsset) {
			if (!LoadedAsset) {
				return;
			}
				

			if (UStaticMesh* LoadedMesh = Cast<UStaticMesh>(LoadedAsset)) {
				Mesh->SetStaticMesh(LoadedMesh);
			}
		}
	));

	QR->SetRelativeLocation(Item->QRPosition);
	FRotator NewRot = QR->GetRelativeRotation();
	NewRot.Pitch += FMath::RandRange(0, 360);
	QR->SetRelativeRotation(NewRot);
	double ScaleQR = FMath::FRandRange(MinQRScale, MaxQRScale);
	QR->SetRelativeScale3D(FVector(ScaleQR, ScaleQR, ScaleQR));

}