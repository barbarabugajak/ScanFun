// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "QRCodeType.h"
#include "ScannerData.generated.h"


USTRUCT(BlueprintType)
struct SCANFUN_API FScannerType
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params")
	FName Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Params", meta=(UIMin=0, ClampMin=0))
	float Cooldown = 0.25f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMesh* ScannerMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	FVector MeshScale = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QRCode", meta = (GetOptions = "ScannableManagementSubsystem.GetQRCodeTypes"))
	FName AssignedQRCode;
};


UCLASS()
class SCANFUN_API UScannerData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Types", meta = (TitleProperty = "Name"))
	TArray<FScannerType> ScannerTypes;
};