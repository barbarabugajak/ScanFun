// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"
#include "ScanAbility.generated.h"

/**
 * 
 */
UCLASS()
class SCANFUN_API UScanAbility : public UGameplayAbilityBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 100, ClampMin = 0, ClampMax = 100))
	int ChanceOfActivation = 0;

};
