// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ScannableDataRow.generated.h"

USTRUCT(BlueprintType)
struct FScannableDataRow : public FTableRowBase {

	GENERATED_BODY()

public:

	FScannableDataRow()
		: QRPosition(FVector::ZeroVector),
		Asset(FSoftObjectPath())
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector QRPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	FSoftObjectPath Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
	float Asset_Scale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
	float commission = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params", meta = (GetOptions = "ScannableManagementSubsystem.GetRarities"))
	FString Rarity;
};