// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Scannable.h"
#include "ConveyorBelt.h"
#include "ScannableSubsystemSettings.generated.h"

/**
 * 
 */
UCLASS(config = Game, DefaultConfig)
class SCANFUN_API UScannableSubsystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UScannableSubsystemSettings(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath QRDataTablePath;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AScannable> ScannableToSpawn_Class;

	// QR 
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly,  Category = "QR Scale")
	double MinQRScale = 0.01f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "QR Scale")
	double MaxQRScale = 0.01f;
};
