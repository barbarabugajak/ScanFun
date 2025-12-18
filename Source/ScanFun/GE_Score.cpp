// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_Score.h"
#include "PlayerBasicAttributeSet.h" 

UGE_Score::UGE_Score()
{
    DurationPolicy = EGameplayEffectDurationType::Instant; 
}

void UGE_Score::PostInitProperties()
{
    Super::PostInitProperties();
}
