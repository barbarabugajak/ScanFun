// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Scannable.h"
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MinQRScale = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QR Scale")
	double MaxQRScale = 0.1f;

};
