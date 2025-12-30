// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RarityDataAsset.generated.h"

/**
 * 
 */
USTRUCT()
struct FRarityDataAssetPart {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Params")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "Cosmetic")
	FColor Color = FColor::Black;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int ProbabilityWeight = 1;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int Cooldown = 1;

	UPROPERTY(EditAnywhere, Category = "Params")
	float Price = 0.0f;

};

UCLASS()
class SCANFUN_API URarityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Rarity")
	TArray<FRarityDataAssetPart> Rarities;
};
