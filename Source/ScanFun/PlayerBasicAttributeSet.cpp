// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBasicAttributeSet.h"

void UPlayerBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) {
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 9.0f, FColor::Yellow, FString::Printf(TEXT("Score Value changes from %f to: %f"), OldValue, NewValue));
}