// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "ScannerData.generated.h"


USTRUCT(BlueprintType)
struct SCANFUN_API FScannerType
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params")
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params")
	FLinearColor Color = FLinearColor::Red;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params", meta=(UIMin=0, ClampMin=0))
	float Cooldown = 0.25f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* ScannerMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	FVector MeshScale = FVector::ZeroVector;
	
};


UCLASS()
class SCANFUN_API UScannerData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Types")
	TArray<FScannerType> ScannerTypes;
};