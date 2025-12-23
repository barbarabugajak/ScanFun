// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Scannable.h"
#include "ConveyorBelt.h"
#include "ScannableManagementSubsystem.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class SCANFUN_API UScannableManagementSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	UPROPERTY()
	TArray<AScannable*> Scannables;

	UFUNCTION()
	void SpawnScannable();

	UFUNCTION()
	void UpdateScannables(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void SetConveyorBeltSetupRelatedVariables(AConveyorBelt* ConveyorBelt);

	UPROPERTY(EditAnywhere)
	UDataTable* QRDataTable;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AScannable> ScannableToSpawn_Class;

	// QR 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MinQRScale = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MaxQRScale = 0.2f;

	// Gameplay Adjustable parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Params")
	float spawnDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Params")
	float objectSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	AConveyorBelt* Belt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	FVector DestructionLocation;

private:
	UPROPERTY()
	float spawnDelayValueCounter = 0.f;
};