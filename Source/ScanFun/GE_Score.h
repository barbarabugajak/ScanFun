// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectBase.h"
#include "GE_Score.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SCANFUN_API UGE_Score : public UGameplayEffectBase
{
    GENERATED_BODY()

public:
    UGE_Score();

protected:
    virtual void PostInitProperties() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTag ScoreSetByCallerTag;


};

// GAS Sanity Check Effect