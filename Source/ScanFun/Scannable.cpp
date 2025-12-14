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

	if (!QRDataTable) return;

	TArray<FName> RowNames = QRDataTable->GetRowNames();

	int i = FMath::RandRange(0, RowNames.Num()-1);

	FQRData* Item = QRDataTable->FindRow<FQRData>(RowNames[i], "");
	
	if (Item->Asset.IsNull()) return;

	FLoadSoftObjectPathAsyncDelegate Delegate;
	Delegate.BindUObject(this, &AScannable::OnMeshAssetLoaded);
	Item->Asset.LoadAsync(Delegate);

	QR->SetRelativeLocation(Item->QRPosition);
}

void AScannable::OnMeshAssetLoaded(const FSoftObjectPath& AssetPath, UObject* LoadedAsset) {
	
	if (!LoadedAsset) return;

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, LoadedAsset->GetName());
	
	if (UStaticMesh* LoadedMesh = Cast<UStaticMesh>(LoadedAsset)) {
		Mesh->SetStaticMesh(LoadedMesh);
	}
	
}
