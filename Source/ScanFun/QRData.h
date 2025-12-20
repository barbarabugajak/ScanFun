// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QRData.generated.h"


/** Structure that defines a datatable entry */
USTRUCT(BlueprintType)
struct FQRData : public FTableRowBase
{
	GENERATED_BODY()

public:

	FQRData()
		: QRPosition(FVector::ZeroVector)
		, Asset(FSoftObjectPath())
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FVector QRPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
	FSoftObjectPath Asset;

};