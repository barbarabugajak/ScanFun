// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Scannable.h"
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
	void UpdateScannables();

	UPROPERTY(EditAnywhere)
	UDataTable* QRDataTable;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AScannable> ScannableToSpawn_Class;

	// QR 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MinQRScale = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MaxQRScale = 0.2f;

private:
	float spawnDelayValue = 15.0f;
};