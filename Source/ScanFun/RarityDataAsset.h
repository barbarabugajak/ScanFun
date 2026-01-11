// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "ScanAbility.h"
#include "Engine/DataAsset.h"
#include "RarityDataAsset.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FRarityDataAssetPart {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Params")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Cosmetic")
	FColor Color = FColor::Black;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int ProbabilityWeight = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int Cooldown = 1;

	UPROPERTY(EditAnywhere, Category = "Params")
	float Price = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<FGameplayTagContainer> ScanAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<FGameplayTagContainer> FailAbilities;

	UPROPERTY(EditAnywhere, Category = "Scanners", meta = (GetOptions = "ScannableManagementSubsystem.GetScannerTypes"))
	TArray<FString> ScannerTypes;

};

UCLASS()
class SCANFUN_API URarityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Rarity", meta = (TitleProperty = "Name"))
	TArray<FRarityDataAssetPart> Rarities;
};
