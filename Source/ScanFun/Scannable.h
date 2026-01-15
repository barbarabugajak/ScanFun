// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QRCodeType.h"
#include "ScannableDataRow.h"
#include "Scannable.generated.h"

UCLASS()
class SCANFUN_API AScannable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScannable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* QR;

	// QR 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR | Params")
	double MinQRScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR | Params")
	double MaxQRScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR | Params")
	FQRCodeTypeEntry QRCodeType;

	UFUNCTION(BlueprintCallable)
	void SetupQRCode(FScannableDataRow Item, FQRCodeTypeEntry QRCodeTypeChosen);

	// Data
	UPROPERTY(EditDefaultsOnly, Category = "DataTable | QR")
	UDataTable* QRDataTable;

	UPROPERTY(EditAnywhere, Category = "Scan")
	bool bWasScanned = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Rarity")
	FName RarityTierName;
};
