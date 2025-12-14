// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QRCodeLocation.generated.h"

USTRUCT(BlueprintType)
struct FQRCodeLocationEntry
{

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AssetScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> Asset;
};

/**
 * 
 */
UCLASS(BlueprintType)
class SCANFUN_API UQRCodeLocation : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQRCodeLocationEntry> Data;
	
};
