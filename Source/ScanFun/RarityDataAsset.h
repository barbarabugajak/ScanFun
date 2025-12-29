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

	UPROPERTY(EditAnywhere, Category = "Rarity")
	FString Name;
	UPROPERTY(EditAnywhere, Category = "Rarity")
	int ProbabilityWeight = 1;
	UPROPERTY(EditAnywhere, Category = "Cosmetic")
	FColor Color = FColor::Black;

};

UCLASS()
class SCANFUN_API URarityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Rarity")
	TArray<FRarityDataAssetPart> Rarities;
};
