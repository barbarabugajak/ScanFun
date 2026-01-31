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
#include "SineHelper.h"
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

	UFUNCTION()
	void SetupInitialScannerBeam();
	
	UPROPERTY()
	bool bWasInitialScannerBeamSetup = false;

	UPROPERTY(EditAnywhere)
	UDataTable* ScannablesData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	URarityDataAsset* RarityDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UScannerData* ScannerDataAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UQRCodeType* QRCodeTypeDataAsset;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AScannable> ScannableToSpawn_Class;
	// Scanners
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FActiveGameplayEffectHandle> ScannerCooldowns;

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
	UPROPERTY(VisibleAnywhere)
	bool bAScanAbilitiesGranted = false;

	// Gameplay
	UFUNCTION()
	void UpdateCooldowns();

	UFUNCTION(BlueprintCallable)
	FRarityDataAssetPart GetRarityTierOfScannable(const AScannable* Scannable);

	UFUNCTION(BlueprintCallable)
	FScannerType GetScannerTypeFromName(const FName Name);

	UFUNCTION(BlueprintCallable)
	FQRCodeTypeEntry GetQRCodeTypeFromName(const FName Name);

	UFUNCTION(BlueprintCallable)
	TArray<FQRCodeTypeEntry> GetQRCodeTypesOfRarityType(const FRarityDataAssetPart RarityTier);

	UFUNCTION(BlueprintCallable)
	FLinearColor GetColorOfScanner(const FScannerType Scanner);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity Tiers")
	TMap<FName, int> Cooldowns;


	UFUNCTION()
	void TryTriggeringRandomFailAbility(int indexOfScannable);

	UFUNCTION()
	void TriggerLoseScore(int indexOfScannable);

	UPROPERTY()
	AActor* Player;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

	// Jittering
	UPROPERTY()
	float jitterCoefficient = 1.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FSineHelper> Sines;

	UPROPERTY()
	bool bIsJittering = false;

	UFUNCTION()
	void CreateRandomSines();

	UFUNCTION()
	void JitterConveyorBelt();


private:
	UPROPERTY()
	float spawnDelayValueCounter = 0.f;

public: 
	UFUNCTION()
	static TArray<FName> GetRarities()
	{
		TArray<FName> RarityNames;

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
	static TArray<FName> GetScannerTypes()
	{
		TArray<FName> ScannerTypes;

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

	UFUNCTION()
	static TArray<FName> GetQRCodeTypes() {
		TArray<FName> QRCodeTypes;

		const UScannableSubsystemSettings* Settings = GetDefault<UScannableSubsystemSettings>();
		if (!Settings)
		{
			return QRCodeTypes;
		}

		UQRCodeType* LoadedDataAsset = Cast<UQRCodeType>(Settings->QRCodeTypesDataAssetPath.TryLoad());

		if (!LoadedDataAsset)
		{
			return QRCodeTypes;
		}

		for (const FQRCodeTypeEntry& Item : LoadedDataAsset->Entries)
		{
			QRCodeTypes.Add(Item.Name);
		}

		return QRCodeTypes;
	}
};