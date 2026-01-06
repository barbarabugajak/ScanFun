// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Scannable.h"
#include "ConveyorBelt.h"
#include "GameplayEffectBase.h"
#include "ScannableSubsystemSettings.h"
#include "RarityDataAsset.h"
#include "ScannableDataRow.h"
#include "ScannerData.h"
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

	UPROPERTY()
	URarityDataAsset* RarityDataAsset;

	UPROPERTY()
	UScannerData* ScannerDataAsset;

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

	// Belt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	AConveyorBelt* Belt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conveyor Belt")
	FVector DestructionLocation;
	
	// Tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer LoseScoreTagContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTag ScannableDestroyedEventTag;

	// Abilities
	UFUNCTION()
	void HaveScanAbilitiesGranted();

	UPROPERTY(VisibleAnywhere)
	bool bAScanAbilitiesGranted = false;

	// Gameplay
	UFUNCTION()
	void UpdateCooldowns();

	UFUNCTION(BlueprintCallable)
	FRarityDataAssetPart GetRarityTierOfScannable(const AScannable* Scannable);

	UFUNCTION(BlueprintCallable)
	FScannerType GetScannerTypeFromName(const FString Name);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity Tiers")
	TMap<FString, int> Cooldowns;
private:
	UPROPERTY()
	float spawnDelayValueCounter = 0.f;

public: 
	UFUNCTION()
	static TArray<FString> GetRarities()
	{
		TArray<FString> RarityNames;

		const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();
		if (!Settings)
		{
			return RarityNames;
		}

		URarityDataAsset* LoadedDataAsset = Cast<URarityDataAsset>(Settings->RarityDataAssetPath.TryLoad());

		if (!LoadedDataAsset)
		{
			return RarityNames;
		}

		for (const FRarityDataAssetPart& Item : LoadedDataAsset->Rarities)
		{
			RarityNames.Add(Item.Name);
		}

		return RarityNames;
	}

	UFUNCTION()
	static TArray<FString> GetScannerTypes()
	{
		TArray<FString> ScannerTypes;

		const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();
		if (!Settings)
		{
			return ScannerTypes;
		}

		UScannerData* LoadedDataAsset = Cast<UScannerData>(Settings->ScannerDataAssetPath.TryLoad());

		if (!LoadedDataAsset)
		{
			return ScannerTypes;
		}

		for (const FScannerType& Item : LoadedDataAsset->ScannerTypes)
		{
			ScannerTypes.Add(Item.Name);
		}

		return ScannerTypes;
	}
};