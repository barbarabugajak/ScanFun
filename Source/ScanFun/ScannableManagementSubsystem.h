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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer LoseScoreTagContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTag ScannableDestroyedEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity Tiers")
	int rarityWeightsSum = 0;

	UFUNCTION()
	FRarityDataAssetPart GetRarityTierOfScannable(const AScannable* Scannable);

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
};