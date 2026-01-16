// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "QRCodeType.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FQRCodeTypeEntry {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params")
	FName Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params")
	FLinearColor Color = FLinearColor::White;
};

UCLASS()
class SCANFUN_API UQRCodeType : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Entries", meta = (TitleProperty = "Name"))
	TArray< FQRCodeTypeEntry> Entries;

	UPROPERTY(EditAnywhere, Category = "Defaults", meta= (GetOptions = "ScannableManagementSubsystem.GetQRCodeTypes"))
	FName DefaultQRCode;
};
