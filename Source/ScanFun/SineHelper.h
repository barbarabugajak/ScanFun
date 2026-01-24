// Attribution: Barbara Bugajak

#pragma once

#include "CoreMinimal.h"
#include "SineHelper.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FSineHelper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float amplitude = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float phase = 0.0f; 

	float CalcValueInPoint(float Position) const
	{
		return amplitude * FMath::Sin(frequency * 2 * PI * (Position + phase));
	}

	static FSineHelper RandomInstance(
		float MinAmplitude = 0.5f, 
		float MaxAmplitude = 1.0f,
		float MinFrequency = 0.5f, 
		float MaxFrequency = 1.0f,
		float MinPhase = 0.0f, 
		float MaxPhase = 1.0f)
	{
		FSineHelper NewHelper;
		NewHelper.amplitude = FMath::FRandRange(MinAmplitude, MaxAmplitude);
		NewHelper.frequency = FMath::FRandRange(MinFrequency, MaxFrequency);
		NewHelper.phase = FMath::FRandRange(MinPhase, MaxPhase);
		return NewHelper;
	}

};

	