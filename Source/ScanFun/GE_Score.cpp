// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_Score.h"
#include "PlayerBasicAttributeSet.h" 

UGE_Score::UGE_Score()
{
    DurationPolicy = EGameplayEffectDurationType::Instant; 
    ScoreSetByCallerTag = FGameplayTag::RequestGameplayTag("Player.Effect.Score");
}

void UGE_Score::PostInitProperties()
{
    Super::PostInitProperties();

    if (!ScoreSetByCallerTag.IsValid()) return;

    FGameplayModifierInfo Modifier;
    Modifier.ModifierOp = EGameplayModOp::AddBase;
    Modifier.Attribute = UPlayerBasicAttributeSet::GetScoreAttribute();
    FSetByCallerFloat SetByCallerData;
    SetByCallerData.DataTag = ScoreSetByCallerTag;
    FGameplayEffectModifierMagnitude GE_MagnitudeModifier(SetByCallerData);
    Modifier.ModifierMagnitude = GE_MagnitudeModifier;
    Modifiers.Add(Modifier);

}
